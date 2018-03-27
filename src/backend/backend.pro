TEMPLATE = lib

QT += qml
CONFIG += c++11 staticlib warn_on exceptions_off


SOURCES += \
    Api.cpp \
    Assets.cpp \
    ConfigFile.cpp \
    DataFinder.cpp \
    FrontendLayer.cpp \
    PegasusAssets.cpp \
    ProcessLauncher.cpp \
    ScriptRunner.cpp \
    Utils.cpp \

HEADERS += \
    Api.h \
    AppCloseType.h \
    Assets.h \
    ConfigFile.h \
    DataFinder.h \
    FrontendLayer.h \
    ListPropertyFn.h \
    PegasusAssets.h \
    ProcessLauncher.h \
    ScriptRunner.h \
    Utils.h \

include(platform/platform.pri)
include(providers/providers.pri)
include(types/types.pri)


include($${TOP_SRCDIR}/src/deployment_vars.pri)

DEFINES *= \
    $${COMMON_DEFINES} \
    INSTALL_DATADIR=\\\"$${INSTALL_DATADIR}\\\"
