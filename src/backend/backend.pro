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
    QuitStatus.cpp \
    ScriptRunner.cpp \
    SystemCommands.cpp \
    Utils.cpp \
    api_parts/ApiMeta.cpp \
    api_parts/ApiSystem.cpp \
    api_parts/ApiSettings.cpp \
    model_providers/Es2Metadata.cpp \
    model_providers/Es2PlatformList.cpp \
    model_providers/PegasusAssets.cpp

HEADERS += \
    Api.h \
    Assets.h \
    DataFinder.h \
    Model.h \
    FrontendLayer.h \
    ProcessLauncher.h \
    QuitStatus.h \
    ScriptRunner.h \
    SystemCommands.h \
    Utils.h \
    api_parts/ApiMeta.h \
    api_parts/ApiSystem.h \
    api_parts/ApiSettings.h \
    model_providers/MetadataProvider.h \
    model_providers/PlatformListProvider.h \
    model_providers/Es2Metadata.h \
    model_providers/Es2PlatformList.h \
    model_providers/PegasusAssets.h


include($${TOP_SRCDIR}/src/deployment_vars.pri)

DEFINES *= \
    QT_DEPRECATED_WARNINGS \
    QT_RESTRICTED_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII \
    GIT_REVISION=\\\"$$GIT_REVISION\\\" \
    INSTALL_DATADIR=\\\"$${INSTALL_DATADIR}\\\"
