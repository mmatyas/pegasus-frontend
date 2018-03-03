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
    SystemCommands.cpp \
    Utils.cpp \
    providers/Provider.cpp \
    providers/es2/Es2Metadata.cpp \
    providers/es2/Es2Provider.cpp \
    providers/es2/Es2Systems.cpp \
    providers/pegasus/PegasusProvider.cpp \
    providers/steam/SteamGamelist.cpp \
    providers/steam/SteamMetadata.cpp \
    providers/steam/SteamProvider.cpp \
    types/Collection.cpp \
    types/CollectionList.cpp \
    types/Filters.cpp \
    types/Game.cpp \
    types/GameAssets.cpp \
    types/GameList.cpp \
    types/Locale.cpp \
    types/LocaleList.cpp \
    types/Meta.cpp \
    types/Settings.cpp \
    types/System.cpp \
    types/Theme.cpp \
    types/ThemeList.cpp

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
    SystemCommands.h \
    Utils.h \
    providers/Provider.h \
    providers/es2/Es2Metadata.h \
    providers/es2/Es2Provider.h \
    providers/es2/Es2Systems.h \
    providers/pegasus/PegasusProvider.h \
    providers/steam/SteamGamelist.h \
    providers/steam/SteamMetadata.h \
    providers/steam/SteamProvider.h \
    types/Collection.h \
    types/CollectionList.h \
    types/Filters.h \
    types/Game.h \
    types/GameAssets.h \
    types/GameList.h \
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
