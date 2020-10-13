add_library(pegasus-backend
    Api.cpp
    Api.h
    AppSettings.cpp
    AppSettings.h
    Backend.cpp
    Backend.h
    CliArgs.h
    FrontendLayer.cpp
    FrontendLayer.h
    GamepadAxisNavigation.cpp
    GamepadAxisNavigation.h
    GamepadButtonNavigation.cpp
    GamepadButtonNavigation.h
    Log.cpp
    Log.h
    Paths.cpp
    Paths.h
    PegasusAssets.cpp
    PegasusAssets.h
    ProcessLauncher.cpp
    ProcessLauncher.h
    ScriptRunner.cpp
    ScriptRunner.h
    imggen/BlurhashProvider.cpp
    imggen/BlurhashProvider.h
    model/gaming/Assets.cpp
    model/gaming/Assets.h
    model/gaming/Collection.cpp
    model/gaming/Collection.h
    model/gaming/Game.cpp
    model/gaming/Game.h
    model/gaming/GameFile.cpp
    model/gaming/GameFile.h
    model/internal/Gamepad.cpp
    model/internal/Gamepad.h
    model/internal/GamepadManager.cpp
    model/internal/GamepadManager.h
    model/internal/GamepadManagerBackend.cpp
    model/internal/GamepadManagerBackend.h
    model/internal/Internal.cpp
    model/internal/Internal.h
    model/internal/Meta.cpp
    model/internal/Meta.h
    model/internal/settings/KeyEditor.cpp
    model/internal/settings/KeyEditor.h
    model/internal/settings/Locales.cpp
    model/internal/settings/Locales.h
    model/internal/settings/Providers.cpp
    model/internal/settings/Providers.h
    model/internal/settings/Settings.cpp
    model/internal/settings/Settings.h
    model/internal/settings/Themes.cpp
    model/internal/settings/Themes.h
    model/internal/System.cpp
    model/internal/System.h
    model/keys/Key.cpp
    model/keys/Key.h
    model/keys/Keys.cpp
    model/keys/Keys.h
    model/memory/Memory.cpp
    model/memory/Memory.h
    parsers/MetaFile.cpp
    parsers/MetaFile.h
    parsers/SettingsFile.cpp
    parsers/SettingsFile.h
    platform/PowerCommands.h
    platform/TerminalKbd.cpp
    platform/TerminalKbd.h
    providers/pegasus_favorites/Favorites.cpp
    providers/pegasus_favorites/Favorites.h
    providers/pegasus_media/MediaProvider.cpp
    providers/pegasus_media/MediaProvider.h
    providers/pegasus_metadata/PegasusFilter.cpp
    providers/pegasus_metadata/PegasusFilter.h
    providers/pegasus_metadata/PegasusMetadata.cpp
    providers/pegasus_metadata/PegasusMetadata.h
    providers/pegasus_metadata/PegasusProvider.cpp
    providers/pegasus_metadata/PegasusProvider.h
    providers/pegasus_playtime/PlaytimeStats.cpp
    providers/pegasus_playtime/PlaytimeStats.h
    providers/Provider.cpp
    providers/Provider.h
    providers/ProviderManager.cpp
    providers/ProviderManager.h
    providers/SearchContext.cpp
    providers/SearchContext.h
    types/AppCloseType.h
    types/AssetType.h
    types/GamepadKeyId.h
    types/KeyEventType.h
    utils/CommandTokenizer.cpp
    utils/CommandTokenizer.h
    utils/DiskCachedNAM.cpp
    utils/DiskCachedNAM.h
    utils/FakeQKeyEvent.cpp
    utils/FakeQKeyEvent.h
    utils/FolderListModel.cpp
    utils/FolderListModel.h
    utils/HashMap.h
    utils/KeySequenceTools.cpp
    utils/KeySequenceTools.h
    utils/MoveOnly.h
    utils/NoCopyNoMove.h
    utils/PathCheck.cpp
    utils/PathCheck.h
    utils/QmlHelpers.h
    utils/SqliteDb.cpp
    utils/SqliteDb.h
    utils/StdHelpers.h
    utils/StdStringHelpers.cpp
    utils/StdStringHelpers.h
    utils/StrBoolConverter.cpp
    utils/StrBoolConverter.h
)

target_include_directories(pegasus-backend PUBLIC
    "${PROJECT_SOURCE_DIR}/thirdparty"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

find_package(Qt5 REQUIRED COMPONENTS
    Qml
    Quick
    Multimedia
    Sql
    Svg
)
target_link_libraries(pegasus-backend PUBLIC
    Qt5::Qml
    Qt5::Quick
    Qt5::Multimedia
    Qt5::Sql
    Qt5::QSvgPlugin
    QtQmlTricks
    SortFilterProxyModel
)

include(PegasusCommonProps)
pegasus_add_common_props(pegasus-backend)


# Select providers

include(PegasusTargetPlatform)
set(PEGASUS_ENABLED_PROVIDERS)
set(PEGASUS_USE_JSON_CACHE OFF)

if(PEGASUS_ON_WINDOWS OR PEGASUS_ON_MACOS OR PEGASUS_ON_X11 OR PEGASUS_ON_EGLFS)
    target_sources(pegasus-backend PRIVATE
        providers/es2/Es2Games.cpp
        providers/es2/Es2Games.h
        providers/es2/Es2Metadata.cpp
        providers/es2/Es2Metadata.h
        providers/es2/Es2Provider.cpp
        providers/es2/Es2Provider.h
        providers/es2/Es2Systems.cpp
        providers/es2/Es2Systems.h
    )
    list(APPEND PEGASUS_ENABLED_PROVIDERS "EmulationStation")
    target_compile_definitions(pegasus-backend PRIVATE WITH_COMPAT_ES2)
endif()
if(PEGASUS_ON_WINDOWS OR PEGASUS_ON_MACOS OR PEGASUS_ON_X11)
    target_sources(pegasus-backend PRIVATE
        providers/steam/SteamGamelist.cpp
        providers/steam/SteamGamelist.h
        providers/steam/SteamMetadata.cpp
        providers/steam/SteamMetadata.h
        providers/steam/SteamProvider.cpp
        providers/steam/SteamProvider.h
    )
    list(APPEND PEGASUS_ENABLED_PROVIDERS "Steam")
    set(PEGASUS_USE_JSON_CACHE ON)
    target_compile_definitions(pegasus-backend PRIVATE WITH_COMPAT_STEAM)
endif()
if(PEGASUS_ON_WINDOWS OR PEGASUS_ON_X11)
    target_sources(pegasus-backend PRIVATE
        providers/gog/GogGamelist.cpp
        providers/gog/GogGamelist.h
        providers/gog/GogMetadata.cpp
        providers/gog/GogMetadata.h
        providers/gog/GogProvider.cpp
        providers/gog/GogProvider.h
    )
    list(APPEND PEGASUS_ENABLED_PROVIDERS "GOG")
    set(PEGASUS_USE_JSON_CACHE ON)
    target_compile_definitions(pegasus-backend PRIVATE WITH_COMPAT_GOG)
endif()
if(PEGASUS_ON_ANDROID)
    target_sources(pegasus-backend PRIVATE
        platform/AndroidAppIconProvider.cpp
        platform/AndroidAppIconProvider.h
        platform/AndroidHelpers.cpp
        platform/AndroidHelpers.h
        providers/android_apps/AndroidAppsMetadata.cpp
        providers/android_apps/AndroidAppsMetadata.h
        providers/android_apps/AndroidAppsProvider.cpp
        providers/android_apps/AndroidAppsProvider.h
    )
    list(APPEND PEGASUS_ENABLED_PROVIDERS "Android Apps")
    target_compile_definitions(pegasus-backend PRIVATE WITH_COMPAT_ANDROIDAPPS)
endif()
if(PEGASUS_ON_WINDOWS)
    target_sources(pegasus-backend PRIVATE
        providers/launchbox/LaunchBoxAssets.cpp
        providers/launchbox/LaunchBoxAssets.h
        providers/launchbox/LaunchBoxEmulator.h
        providers/launchbox/LaunchBoxEmulatorsXml.cpp
        providers/launchbox/LaunchBoxEmulatorsXml.h
        providers/launchbox/LaunchBoxGamelistXml.cpp
        providers/launchbox/LaunchBoxGamelistXml.h
        providers/launchbox/LaunchBoxPlatformsXml.cpp
        providers/launchbox/LaunchBoxPlatformsXml.h
        providers/launchbox/LaunchBoxProvider.cpp
        providers/launchbox/LaunchBoxProvider.h
        providers/launchbox/LaunchBoxXml.cpp
        providers/launchbox/LaunchBoxXml.h
    )
    list(APPEND PEGASUS_ENABLED_PROVIDERS "LaunchBox")
    target_compile_definitions(pegasus-backend PRIVATE WITH_COMPAT_LAUNCHBOX)
endif()
if(PEGASUS_ON_X11)
    target_sources(pegasus-backend PRIVATE
        providers/lutris/LutrisProvider.cpp
        providers/lutris/LutrisProvider.h
    )
    list(APPEND PEGASUS_ENABLED_PROVIDERS "Lutris")
    target_compile_definitions(pegasus-backend PRIVATE WITH_COMPAT_LUTRIS)
endif()
if(TRUE)
    target_sources(pegasus-backend PRIVATE
        providers/skraper/SkraperAssetsProvider.cpp
        providers/skraper/SkraperAssetsProvider.h
    )
    target_compile_definitions(pegasus-backend PRIVATE WITH_COMPAT_SKRAPER)
    list(APPEND PEGASUS_ENABLED_PROVIDERS "Skraper")

    target_sources(pegasus-backend PRIVATE
        providers/logiqx/LogiqxProvider.cpp
        providers/logiqx/LogiqxProvider.h
    )
    target_compile_definitions(pegasus-backend PRIVATE WITH_COMPAT_LOGIQX)
    list(APPEND PEGASUS_ENABLED_PROVIDERS "Logiqx")
endif()

if(PEGASUS_USE_JSON_CACHE)
    target_sources(pegasus-backend PRIVATE
        providers/JsonCacheUtils.cpp
        providers/JsonCacheUtils.h
    )
endif()

message("Third-party data sources:")
list(SORT PEGASUS_ENABLED_PROVIDERS)
foreach(provider IN LISTS PEGASUS_ENABLED_PROVIDERS)
    message("  - ${provider}")
endforeach()


# Select power manager implementation

message("Power management:")
if(PEGASUS_ON_X11 OR PEGASUS_ON_EGLFS)
    target_sources(pegasus-backend PRIVATE platform/PowerCommands_linux.cpp)
    message("  - Linux")
elseif(PEGASUS_ON_WINDOWS)
    target_sources(pegasus-backend PRIVATE platform/PowerCommands_win.cpp)
    message("  - Windows")
elseif(PEGASUS_ON_MACOS)
    target_sources(pegasus-backend PRIVATE platform/PowerCommands_mac.cpp)
    message("  - macOS")
else()
    target_sources(pegasus-backend PRIVATE platform/PowerCommands_unimpl.cpp)
    message("  - unavailable")
endif()


# Select gamepad implementation

message("Gamepad implementation:")
if(PEGASUS_USE_SDL2_GAMEPAD)
    target_sources(pegasus-backend PRIVATE
        model/internal/GamepadManagerSDL2.cpp
        model/internal/GamepadManagerSDL2.h
    )
    find_package(SDL2 REQUIRED)
    target_include_directories(pegasus-backend PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(pegasus-backend PUBLIC ${SDL2_LIBRARIES})
    target_compile_definitions(pegasus-backend PRIVATE
        WITH_SDL_GAMEPAD
        SDL_MAIN_HANDLED
    )
    message("  - SDL2")
else()
    target_sources(pegasus-backend PRIVATE
        model/internal/GamepadManagerQt.cpp
        model/internal/GamepadManagerQt.h
    )
    find_package(Qt5 REQUIRED COMPONENTS Gamepad)
    target_link_libraries(pegasus-backend PUBLIC Qt5::Gamepad)
    message("  - Qt")
endif()
