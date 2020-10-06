TEMPLATE = lib

QT += qml quick sql
CONFIG += c++11 staticlib warn_on exceptions_off
android: QT += androidextras

!isEmpty(USE_SDL_GAMEPAD): include($${TOP_SRCDIR}/src/link_to_sdl.pri)
else: QT += gamepad

!isEmpty(INSIDE_FLATPAK): DEFINES *= PEGASUS_INSIDE_FLATPAK
msvc: DEFINES *= _USE_MATH_DEFINES


SOURCES += \
    Api.cpp \
    Backend.cpp \
    FrontendLayer.cpp \
    GamepadAxisNavigation.cpp \
    PegasusAssets.cpp \
    PreInit.cpp \
    ProcessLauncher.cpp \
    ScriptRunner.cpp \
    Paths.cpp \
    AppSettings.cpp \
    Log.cpp \
    GamepadButtonNavigation.cpp

HEADERS += \
    Api.h \
    Backend.h \
    CliArgs.h \
    FrontendLayer.h \
    GamepadAxisNavigation.h \
    PegasusAssets.h \
    PreInit.h \
    ProcessLauncher.h \
    ScriptRunner.h \
    LocaleUtils.h \
    Paths.h \
    AppSettings.h \
    Log.h \
    GamepadButtonNavigation.h

include(imggen/imggen.pri)
include(model/model.pri)
include(parsers/parsers.pri)
include(platform/platform.pri)
include(providers/providers.pri)
include(types/types.pri)
include(utils/utils.pri)

DEFINES *= $${COMMON_DEFINES}

include($${TOP_SRCDIR}/thirdparty/thirdparty.pri)
