include($${TOP_SRCDIR}/src/deployment_vars.pri)


!isEmpty(INSTALL_DOCDIR) {
    md.files += \
        $${TOP_SRCDIR}/LICENSE.md \
        $${TOP_SRCDIR}/README.md
    md.path = $${INSTALL_DOCDIR}
    OTHER_FILES += $${icon.files}
    INSTALLS += md
}

unix:!macx {
    target.path = $${INSTALL_BINDIR}

    !isEmpty(INSTALL_ICONDIR) {
        icon.files += platform/linux/org.pegasus_frontend.Pegasus.png
        icon.path = $${INSTALL_ICONDIR}
        OTHER_FILES += $${icon.files}
        INSTALLS += icon
    }
    !isEmpty(INSTALL_DESKTOPDIR) {
        desktop_file.input = platform/linux/org.pegasus_frontend.Pegasus.desktop.in
        desktop_file.output = $${OUT_PWD}/org.pegasus_frontend.Pegasus.desktop
        OTHER_FILES += $${desktop_file.input}

        QMAKE_SUBSTITUTES += desktop_file
        desktop.files += $$desktop_file.output
        desktop.path = $${INSTALL_DESKTOPDIR}
        INSTALLS += desktop
    }
    !isEmpty(INSTALL_APPSTREAMDIR) {
        appstream.files += platform/linux/org.pegasus_frontend.Pegasus.metainfo.xml
        appstream.path = $${INSTALL_APPSTREAMDIR}
        OTHER_FILES += $${appstream.files}
        INSTALLS += appstream
    }
}
win32 {
    QMAKE_TARGET_PRODUCT = "pegasus-frontend"
    QMAKE_TARGET_COMPANY = "pegasus-frontend.org"
    QMAKE_TARGET_DESCRIPTION = "Pegasus emulator frontend"
    QMAKE_TARGET_COPYRIGHT = "Copyright (c) 2017-2020 Matyas Mustoha"
    RC_ICONS = platform/windows/app_icon.ico
    OTHER_FILES += $${RC_ICONS}

    target.path = $${INSTALL_BINDIR}
}
macx {
    ICON = platform/macos/pegasus-fe.icns
    QMAKE_APPLICATION_BUNDLE_NAME = Pegasus
    QMAKE_TARGET_BUNDLE_PREFIX = org.pegasus-frontend
    QMAKE_INFO_PLIST = platform/macos/Info.plist.in

    target.path = $${INSTALL_BINDIR}
}
android {
    QT += androidextras
    OTHER_FILES += \
        platform/android/AndroidManifest.xml \
        platform/android/res/drawable/banner.png \
        platform/android/res/drawable-ldpi/icon.png \
        platform/android/res/drawable-mdpi/icon.png \
        platform/android/res/drawable-hdpi/icon.png \
        platform/android/res/drawable-xhdpi/icon.png \
        platform/android/res/drawable-xxhdpi/icon.png \
        platform/android/res/drawable-xxxhdpi/icon.png \
        platform/android/src/org/pegasus_frontend/android/MainActivity.java \
        platform/android/res/values/libs.xml \

    ANDROID_MIN_SDK_VERSION = 21
    ANDROID_TARGET_SDK_VERSION = 22

    ANDROID_CFGDIR_IN = $$PWD/platform/android
    ANDROID_CFGDIR_OUT = $$OUT_PWD/android
    equals(ANDROID_CFGDIR_IN, $${ANDROID_CFGDIR_OUT}) {
        ANDROID_CFGDIR_OUT = $$OUT_PWD/android.out
    }
    ANDROID_MANIFEST_OUT = $${ANDROID_CFGDIR_OUT}/AndroidManifest.xml

    # TODO: make this cross-platform
    QMAKE_POST_LINK += \
        $${QMAKE_COPY_DIR} $$shell_path($$ANDROID_CFGDIR_IN) $$shell_path($$ANDROID_CFGDIR_OUT) && \
        sed -i s/@GIT_REVISION@/$${GIT_REVISION}/ $$shell_path($$ANDROID_MANIFEST_OUT) && \
        sed -i s/@GIT_COMMIT_CNT@/$${GIT_COMMIT_CNT}/ $$shell_path($$ANDROID_MANIFEST_OUT)

    ANDROID_PACKAGE_SOURCE_DIR = $${ANDROID_CFGDIR_OUT}
    ANDROID_EXTRA_LIBS += \
        /opt/openssl-111d_android/lib/libcrypto.so \
        /opt/openssl-111d_android/lib/libssl.so
}


!isEmpty(target.path): INSTALLS += target
