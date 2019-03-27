TEMPLATE = lib

QT += qml quick gamepad sql
CONFIG += c++11 staticlib warn_on exceptions_off
android: QT += androidextras


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
    Log.cpp

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
