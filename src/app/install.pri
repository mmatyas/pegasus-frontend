include($${TOP_SRCDIR}/src/deployment_vars.pri)


!isEmpty(INSTALL_DOCDIR) {
    md.files += \
        $${TOP_SRCDIR}/LICENSE.md \
        $${TOP_SRCDIR}/README.md
    md.path = $${INSTALL_DOCDIR}
    OTHER_FILES += $${md.files}
    INSTALLS += md
}

unix:!macx {
    target.path = $${INSTALL_BINDIR}

    !isEmpty(INSTALL_ICONDIR) {
        icon16.files += platform/linux/icons/16/org.pegasus_frontend.Pegasus.png
        icon32.files += platform/linux/icons/32/org.pegasus_frontend.Pegasus.png
        icon48.files += platform/linux/icons/48/org.pegasus_frontend.Pegasus.png
        icon64.files += platform/linux/icons/64/org.pegasus_frontend.Pegasus.png
        icon128.files += platform/linux/icons/128/org.pegasus_frontend.Pegasus.png

        icon16.path = $${INSTALL_ICONDIR}/16x16/apps/
        icon32.path = $${INSTALL_ICONDIR}/32x32/apps/
        icon48.path = $${INSTALL_ICONDIR}/48x48/apps/
        icon64.path = $${INSTALL_ICONDIR}/64x64/apps/
        icon128.path = $${INSTALL_ICONDIR}/128x128/apps/

        INSTALLS += icon16 icon32 icon48 icon64 icon128
        OTHER_FILES += $${icon16.files} $${icon32.files} $${icon48.files} $${icon64.files} $${icon128.files}
    }
    !isEmpty(INSTALL_DESKTOPDIR) {
        desktop_file.input = platform/linux/org.pegasus_frontend.Pegasus.desktop.qmake.in
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
        platform/android/res/mipmap-ldpi/icon.png \
        platform/android/res/mipmap-mdpi/icon.png \
        platform/android/res/mipmap-hdpi/icon.png \
        platform/android/res/mipmap-xhdpi/icon.png \
        platform/android/res/mipmap-xxhdpi/icon.png \
        platform/android/res/mipmap-xxxhdpi/icon.png \
        platform/android/res/mipmap-ldpi/icon_fg.png \
        platform/android/res/mipmap-mdpi/icon_fg.png \
        platform/android/res/mipmap-hdpi/icon_fg.png \
        platform/android/res/mipmap-xhdpi/icon_fg.png \
        platform/android/res/mipmap-xxhdpi/icon_fg.png \
        platform/android/res/mipmap-xxxhdpi/icon_fg.png \
        platform/android/res/mipmap-anydpi-v26/icon.xml \
        platform/android/res/mipmap-xhdpi/banner.png \
        platform/android/src/org/pegasus_frontend/android/App.java \
        platform/android/src/org/pegasus_frontend/android/BatteryInfo.java \
        platform/android/src/org/pegasus_frontend/android/IntentHelper.java \
        platform/android/src/org/pegasus_frontend/android/LaunchFileProvider.java \
        platform/android/src/org/pegasus_frontend/android/MainActivity.java \
        platform/android/res/values/colors.xml \
        platform/android/res/values/themes.xml \
        platform/android/res/values/libs.xml \

    ANDROID_MIN_SDK_VERSION = 21
    ANDROID_TARGET_SDK_VERSION = 30

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
        /opt/openssl-111t_android-arm/lib/libcrypto.so \
        /opt/openssl-111t_android-arm/lib/libssl.so \
        /opt/openssl-111t_android-arm64/lib/libcrypto.so \
        /opt/openssl-111t_android-arm64/lib/libssl.so \
        /opt/openssl-111t_android-x86/lib/libcrypto.so \
        /opt/openssl-111t_android-x86/lib/libssl.so \
        /opt/openssl-111t_android-x86_64/lib/libcrypto.so \
        /opt/openssl-111t_android-x86_64/lib/libssl.so
}


!isEmpty(target.path): INSTALLS += target
