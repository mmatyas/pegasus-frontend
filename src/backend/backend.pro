TEMPLATE = lib

QT += qml
CONFIG += c++11 staticlib warn_on exceptions_off rtti_off


SOURCES += \
    Api.cpp \
    Assets.cpp \
    DataFinder.cpp \
    FrontendLayer.cpp \
    Model.cpp \
    ProcessLauncher.cpp \
    ScriptRunner.cpp \
    SystemCommands.cpp \
    Utils.cpp \
    api_parts/ApiFilters.cpp \
    api_parts/ApiMeta.cpp \
    api_parts/ApiSettings.cpp \
    api_parts/ApiSystem.cpp \
    model_providers/Es2Metadata.cpp \
    model_providers/Es2PlatformList.cpp \
    model_providers/PegasusAssets.cpp \
    api_parts/ApiPlatforms.cpp \
    model/Language.cpp \
    model/ThemeEntry.cpp

HEADERS += \
    Api.h \
    Assets.h \
    DataFinder.h \
    Model.h \
    FrontendLayer.h \
    ProcessLauncher.h \
    ScriptRunner.h \
    SystemCommands.h \
    Utils.h \
    api_parts/ApiFilters.h \
    api_parts/ApiMeta.h \
    api_parts/ApiSettings.h \
    api_parts/ApiSystem.h \
    model_providers/MetadataProvider.h \
    model_providers/PlatformListProvider.h \
    model_providers/Es2Metadata.h \
    model_providers/Es2PlatformList.h \
    model_providers/PegasusAssets.h \
    api_parts/ApiPlatforms.h \
    AppCloseType.h \
    model/Language.h \
    model/ThemeEntry.h


include($${TOP_SRCDIR}/src/deployment_vars.pri)

DEFINES *= \
    $${COMMON_DEFINES} \
    INSTALL_DATADIR=\\\"$${INSTALL_DATADIR}\\\"
