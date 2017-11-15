TEMPLATE = lib

QT += qml
CONFIG += c++11 staticlib warn_on exceptions_off


SOURCES += \
    Api.cpp \
    Assets.cpp \
    DataFinder.cpp \
    FrontendLayer.cpp \
    ProcessLauncher.cpp \
    ScriptRunner.cpp \
    SystemCommands.cpp \
    Utils.cpp \
    model_providers/Es2Metadata.cpp \
    model_providers/Es2PlatformList.cpp \
    model_providers/PegasusAssets.cpp \
    model_providers/AppFiles.cpp \
    model_providers/SteamPlatform.cpp \
    model_providers/SteamMetadata.cpp \
    types/Game.cpp \
    types/GameAssets.cpp \
    types/GameList.cpp \
    types/Filters.cpp \
    types/Locale.cpp \
    types/LocaleList.cpp \
    types/Meta.cpp \
    types/Platform.cpp \
    types/PlatformList.cpp \
    types/Settings.cpp \
    types/System.cpp \
    types/Theme.cpp \
    types/ThemeList.cpp

HEADERS += \
    Api.h \
    Assets.h \
    DataFinder.h \
    FrontendLayer.h \
    ProcessLauncher.h \
    ScriptRunner.h \
    SystemCommands.h \
    Utils.h \
    model_providers/MetadataProvider.h \
    model_providers/PlatformListProvider.h \
    model_providers/Es2Metadata.h \
    model_providers/Es2PlatformList.h \
    model_providers/PegasusAssets.h \
    AppCloseType.h \
    model_providers/AppFiles.h \
    ListPropertyFn.h \
    model_providers/SteamPlatform.h \
    model_providers/SteamMetadata.h \
    types/Filters.h \
    types/Game.h \
    types/GameAssets.h \
    types/GameList.h \
    types/Platform.h \
    types/PlatformList.h \
    types/Locale.h \
    types/LocaleList.h \
    types/Meta.h \
    types/Settings.h \
    types/System.h \
    types/Theme.h \
    types/ThemeList.h


include($${TOP_SRCDIR}/src/deployment_vars.pri)

DEFINES *= \
    $${COMMON_DEFINES} \
    INSTALL_DATADIR=\\\"$${INSTALL_DATADIR}\\\"
