TEMPLATE = lib

QT += qml quick sql
CONFIG += c++11 staticlib warn_on exceptions_off
android: QT += androidextras

!isEmpty(USE_SDL_GAMEPAD): include($${TOP_SRCDIR}/src/link_to_sdl.pri)
else: QT += gamepad


SOURCES += \
    Api.cpp \
    AppContext.cpp \
    Backend.cpp \
    FrontendLayer.cpp \
    GamepadAxisNavigation.cpp \
    PegasusAssets.cpp \
    ProcessLauncher.cpp \
    ScriptRunner.cpp \
    Paths.cpp \
    AppSettings.cpp \
    Log.cpp \
    GamepadButtonNavigation.cpp

HEADERS += \
    Api.h \
    AppContext.h \
    Backend.h \
    FrontendLayer.h \
    GamepadAxisNavigation.h \
    PegasusAssets.h \
    ProcessLauncher.h \
    ScriptRunner.h \
    LocaleUtils.h \
    Paths.h \
    AppSettings.h \
    Log.h \
    GamepadButtonNavigation.h

include(parsers/parsers.pri)
include(platform/platform.pri)
include(providers/providers.pri)
include(model/model.pri)
include(modeldata/modeldata.pri)
include(utils/utils.pri)
include(types/types.pri)


DEFINES *= \
    $${COMMON_DEFINES} \
    INSTALL_DATADIR=\\\"$${INSTALL_DATADIR}\\\"

include($${TOP_SRCDIR}/thirdparty/thirdparty.pri)
