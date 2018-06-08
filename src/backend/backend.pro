TEMPLATE = lib

QT += qml gamepad
CONFIG += c++11 staticlib warn_on exceptions_off object_parallel_to_source


SOURCES += \
    Api.cpp \
    AppContext.cpp \
    Assets.cpp \
    Backend.cpp \
    ConfigFile.cpp \
    DataFinder.cpp \
    FrontendLayer.cpp \
    GamepadAxisNavigation.cpp \
    PegasusAssets.cpp \
    ProcessLauncher.cpp \
    ScriptRunner.cpp \
    Utils.cpp \
    Paths.cpp \
    AppArgs.cpp \

HEADERS += \
    Api.h \
    AppCloseType.h \
    AppContext.h \
    Assets.h \
    Backend.h \
    ConfigFile.h \
    DataFinder.h \
    FrontendLayer.h \
    GamepadAxisNavigation.h \
    ListPropertyFn.h \
    PegasusAssets.h \
    ProcessLauncher.h \
    ScriptRunner.h \
    Utils.h \
    LocaleUtils.h \
    Paths.h \
    AppArgs.h \
    QStringHash.h

include(configfiles/configfiles.pri)
include(platform/platform.pri)
include(providers/providers.pri)
include(model/model.pri)
include(modeldata/modeldata.pri)


include($${TOP_SRCDIR}/src/deployment_vars.pri)

DEFINES *= \
    $${COMMON_DEFINES} \
    INSTALL_DATADIR=\\\"$${INSTALL_DATADIR}\\\"
