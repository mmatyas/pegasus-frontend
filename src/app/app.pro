TEMPLATE = app

TARGET = pegasus-fe

QT += qml quick multimedia gamepad svg
CONFIG += c++11 warn_on exceptions_off

win32: LIBS += -luser32 -ladvapi32
macx: LIBS += -framework Cocoa

SOURCES += main.cpp

RESOURCES += \
    lang/translations.qrc \
    ../qmlutils/qmlutils.qrc \
    ../frontend/frontend.qrc \
    ../themes/themes.qrc \
    $${TOP_SRCDIR}/assets/assets.qrc

DEFINES *= $${COMMON_DEFINES}


# Linking

include($${TOP_SRCDIR}/src/link_to_backend.pri)


# Translations

LOCALE_QRC_FILE = $${TOP_SRCDIR}/lang/translations.qrc.in
LOCALE_TS_FILES = $$files($${TOP_SRCDIR}/lang/pegasus_*.ts)

# build the QM files
qtPrepareTool(LRELEASE, lrelease)
locales.name = Translations
locales.input = LOCALE_TS_FILES
locales.output  = lang/${QMAKE_FILE_BASE}.qm
locales.commands = $$LRELEASE -removeidentical ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
locales.clean = ${QMAKE_FILE_OUT}
locales.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += locales

# copy the QRC only after all QM files have been built
for(tsfile, LOCALE_TS_FILES) {
    qmfile = lang/$$basename(tsfile)
    qmfile ~= s/.ts$/.qm

    LOCALE_QRC_DEPS += $$qmfile
}

# copy the QRC file
locales_qrc.name = Translations QRC
locales_qrc.input = LOCALE_QRC_FILE
locales_qrc.output  = lang/${QMAKE_FILE_BASE}
locales_qrc.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
locales_qrc.clean = ${QMAKE_FILE_OUT}
locales_qrc.depends = $${LOCALE_QRC_DEPS}
locales_qrc.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += locales_qrc


# Deployment

include($${TOP_SRCDIR}/src/deployment_vars.pri)

unix:!macx {
    target.path = $${INSTALL_BINDIR}

    !isEmpty(INSTALL_ICONDIR) {
        icon.files += $${TOP_SRCDIR}/etc/linux/pegasus-frontend.png
        icon.path = $${INSTALL_ICONDIR}
        INSTALLS += icon
    }
    !isEmpty(INSTALL_DESKTOPDIR) {
        desktop_file.input = $${TOP_SRCDIR}/etc/linux/pegasus-frontend.desktop.in
        desktop_file.output = $${OUT_PWD}/pegasus-frontend.desktop
        QMAKE_SUBSTITUTES += desktop_file
        desktop.files += $$desktop_file.output
        desktop.path = $${INSTALL_DESKTOPDIR}
        INSTALLS += desktop
    }
}
win32 {
    QMAKE_TARGET_PRODUCT = "pegasus-frontend"
    QMAKE_TARGET_COMPANY = "pegasus-frontend.org"
    QMAKE_TARGET_DESCRIPTION = "Pegasus emulator frontend"
    QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2017-2018 Matyas Mustoha"
    RC_ICONS = $${TOP_SRCDIR}/etc/windows/app_icon.ico

    target.path = $${INSTALL_BINDIR}
}
android {
    DISTFILES += \
        android/AndroidManifest.xml \
        android/res/drawable-hdpi/icon.png \
        android/res/drawable-ldpi/icon.png \
        android/res/drawable-mdpi/icon.png \
        android/res/values/libs.xml \

    ANDROID_CFGDIR_IN = $$PWD/android
    ANDROID_CFGDIR_OUT = $$OUT_PWD/android
    equals(ANDROID_CFGDIR_IN, $${ANDROID_CFGDIR_OUT}) {
        ANDROID_CFGDIR_OUT = $$OUT_PWD/android.out
    }

    # TODO: make this cross-platform
    QMAKE_POST_LINK += \
        $${QMAKE_COPY_DIR} $$shell_path($$ANDROID_CFGDIR_IN) $$shell_path($$ANDROID_CFGDIR_OUT) && \
        sed -i s/@GIT_REVISION@/$${GIT_REVISION}/ $$shell_path($${ANDROID_CFGDIR_OUT}/AndroidManifest.xml)

    ANDROID_PACKAGE_SOURCE_DIR = $${ANDROID_CFGDIR_OUT}
}

!isEmpty(target.path): INSTALLS += target
