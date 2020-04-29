# Link the project that includes this file to the Backend

QT *= qml quick multimedia svg sql

win32: LIBS += -luser32 -ladvapi32
macx: LIBS += -framework Cocoa


# based on the auto-generated code by Qt Creator

win32:CONFIG(release, debug|release): LIBS += "-L$${TOP_BUILDDIR}/src/backend/release/" -lbackend
else:win32:CONFIG(debug, debug|release): LIBS += "-L$${TOP_BUILDDIR}/src/backend/debug/" -lbackend
else:unix: LIBS += "-L$${TOP_BUILDDIR}/src/backend/" -lbackend

INCLUDEPATH += "$${TOP_SRCDIR}/src" "$${TOP_SRCDIR}/src/backend" "$${TOP_SRCDIR}/src/components" "$${TOP_SRCDIR}/thirdparty"
DEPENDPATH += "$${TOP_SRCDIR}/src/backend" "$${TOP_SRCDIR}/src/components" "$${TOP_SRCDIR}/thirdparty"

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/src/backend/release/libbackend.a"
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/src/backend/debug/libbackend.a"
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/src/backend/release/backend.lib"
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += "$${TOP_BUILDDIR}/src/backend/debug/backend.lib"
else:unix: PRE_TARGETDEPS += "$${TOP_BUILDDIR}/src/backend/libbackend.a"


# Gamepad backend
!isEmpty(USE_SDL_GAMEPAD): include($${TOP_SRCDIR}/src/link_to_sdl.pri)
else: QT *= gamepad
