# Link the project that includes this file to the Backend

QT *= qml quick multimedia svg sql

win32: LIBS += -luser32 -ladvapi32
macx: LIBS += -framework Cocoa


# based on the auto-generated code by Qt Creator

win32:CONFIG(release, debug|release): LIBS += "-L$${TOP_BUILDDIR}/src/backend/release/"
else:win32:CONFIG(debug, debug|release): LIBS += "-L$${TOP_BUILDDIR}/src/backend/debug/"
else:unix: LIBS += "-L$${TOP_BUILDDIR}/src/backend/"

LIBS += -lbackend
INCLUDEPATH += \
    "$${TOP_SRCDIR}/src" \
    "$${TOP_SRCDIR}/src/backend" \
    "$${TOP_SRCDIR}/thirdparty"
DEPENDPATH += \
    "$${TOP_SRCDIR}/src/backend" \
    "$${TOP_SRCDIR}/thirdparty"

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/src/backend/release/libbackend.a"
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/src/backend/debug/libbackend.a"
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/src/backend/release/backend.lib"
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/src/backend/debug/backend.lib"
else:unix: PRE_TARGETDEPS += "$${TOP_BUILDDIR}/src/backend/libbackend.a"


# SDL2
!isEmpty(USE_SDL_GAMEPAD)|!isEmpty(USE_SDL_POWER): include($${TOP_SRCDIR}/thirdparty/link_to_sdl.pri)

# Gamepad backend
isEmpty(USE_SDL_GAMEPAD): QT *= gamepad


# Plugins

!isEmpty(ENABLE_APNG) {
    DEFINES *= WITH_APNG_SUPPORT

    win32:CONFIG(release, debug|release): LIBS += "-L$${TOP_BUILDDIR}/thirdparty/apng/release/"
    else:win32:CONFIG(debug, debug|release): LIBS += "-L$${TOP_BUILDDIR}/thirdparty/apng/debug/"
    else:unix: LIBS += "-L$${TOP_BUILDDIR}/thirdparty/apng/"
    LIBS += -lqapng

    win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/thirdparty/apng/release/libqapng.a"
    else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/thirdparty/apng/debug/libqapng.a"
    else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/thirdparty/apng/release/qapng.lib"
    else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/thirdparty/apng/debug/qapng.lib"
}

# NOTE: This is a workaround for the Android CI, where qmake doesn't link Qt's PNG otherwise
!isEmpty(FORCE_QT_PNG):android {
    contains(QT_ARCH, ".*armeabi.*") {
        LIBS += "-L$$[QT_INSTALL_LIBS]" -lqtlibpng_armeabi-v7a
        PRE_TARGETDEPS += "$$[QT_INSTALL_LIBS]/libqtlibpng_armeabi-v7a.a"
    }
    contains(QT_ARCH, ".*arm64.*") {
        LIBS += "-L$$[QT_INSTALL_LIBS]" -lqtlibpng_arm64-v8a
        PRE_TARGETDEPS += "$$[QT_INSTALL_LIBS]/libqtlibpng_arm64-v8a.a"
    }
}
