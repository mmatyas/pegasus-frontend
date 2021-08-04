include(PegasusTargetPlatform)
include(PegasusQtUtils)


add_library(pegasus-backend
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
    model/Api.cpp
    model/Api.h
    model/device/DeviceInfo.cpp
    model/device/DeviceInfo.h
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
    model/internal/System.cpp
    model/internal/System.h
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
    providers/Provider.cpp
    providers/Provider.h
    providers/ProviderManager.cpp
    providers/ProviderManager.h
    providers/ProviderUtils.cpp
    providers/ProviderUtils.h
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
    utils/PathTools.cpp
    utils/PathTools.h
    utils/QmlHelpers.h
    utils/SqliteDb.cpp
    utils/SqliteDb.h
    utils/StdHelpers.h
    utils/StdStringHelpers.cpp
    utils/StdStringHelpers.h
    utils/StrBoolConverter.cpp
    utils/StrBoolConverter.h
)
if(PEGASUS_ON_ANDROID)
    target_sources(pegasus-backend PRIVATE
        platform/AndroidAppIconProvider.cpp
        platform/AndroidAppIconProvider.h
        platform/AndroidHelpers.cpp
        platform/AndroidHelpers.h
    )
endif()

target_include_directories(pegasus-backend PUBLIC
    "${PROJECT_SOURCE_DIR}/thirdparty"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

pegasus_require_qt(COMPONENTS
    Qml
    Quick
    Multimedia
    Sql
    Svg
)
target_link_libraries(pegasus-backend PUBLIC
    Qt::Qml
    Qt::Quick
    Qt::Multimedia
    Qt::Sql
    Qt::Svg
    QtQmlTricks
    SortFilterProxyModel
)

include(PegasusCommonProps)
pegasus_add_common_props_optimized(pegasus-backend)


# Select providers

set(OPTIONAL_PROVIDERS CACHE INTERNAL "")
set(USE_JSON_CACHE OFF CACHE INTERNAL "")


function(pegasus_add_provider)
    cmake_parse_arguments(PROVIDER
        "INTERNAL;USES_JSON_CACHE"
        "NAME;CXXID"
        "SOURCES;PLATFORMS"
        ${ARGN})

    set(PROVIDER_ENABLED OFF)
    if("ALL" IN_LIST PROVIDER_PLATFORMS)
        set(PROVIDER_ENABLED ON)
    else()
        foreach(PLATFORM IN LISTS PROVIDER_PLATFORMS)
            if(DEFINED "PEGASUS_ON_${PLATFORM}")
                set(PROVIDER_ENABLED ON)
            endif()
        endforeach()
    endif()

    if(PROVIDER_ENABLED)
        list(TRANSFORM PROVIDER_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
        target_sources(pegasus-backend PRIVATE "${PROVIDER_SOURCES}")
        if(PROVIDER_CXXID)
            target_compile_definitions(pegasus-backend PRIVATE "WITH_COMPAT_${PROVIDER_CXXID}")
        endif()

        if(PROVIDER_USES_JSON_CACHE)
            set(USE_JSON_CACHE ON CACHE INTERNAL "")
        endif()

        if(NOT PROVIDER_INTERNAL)
            list(APPEND OPTIONAL_PROVIDERS "${PROVIDER_NAME}")
            set(OPTIONAL_PROVIDERS "${OPTIONAL_PROVIDERS}" CACHE INTERNAL "")
        endif()
    endif()
endfunction()


add_subdirectory(providers/android_apps)
add_subdirectory(providers/es2)
add_subdirectory(providers/gog)
add_subdirectory(providers/launchbox)
add_subdirectory(providers/logiqx)
add_subdirectory(providers/lutris)
add_subdirectory(providers/pegasus_favorites)
add_subdirectory(providers/pegasus_media)
add_subdirectory(providers/pegasus_metadata)
add_subdirectory(providers/pegasus_playtime)
add_subdirectory(providers/playnite)
add_subdirectory(providers/skraper)
add_subdirectory(providers/steam)

if(USE_JSON_CACHE)
    target_compile_definitions(pegasus-backend PRIVATE WITH_JSON_CACHE)
endif()

set(USE_SDL2 OFF)


# Select power manager implementation

if(PEGASUS_ON_X11 OR PEGASUS_ON_EGLFS)
    target_sources(pegasus-backend PRIVATE platform/PowerCommands_linux.cpp)
    set(POWER_IMPL_NAME "Linux")
elseif(PEGASUS_ON_WINDOWS)
    target_sources(pegasus-backend PRIVATE platform/PowerCommands_win.cpp)
    set(POWER_IMPL_NAME "Windows")
elseif(PEGASUS_ON_MACOS)
    target_sources(pegasus-backend PRIVATE platform/PowerCommands_mac.cpp)
    set(POWER_IMPL_NAME "macOS")
else()
    target_sources(pegasus-backend PRIVATE platform/PowerCommands_unimpl.cpp)
    set(POWER_IMPL_NAME "Disabled")
endif()


# Select gamepad implementation

if(PEGASUS_USE_SDL2_GAMEPAD)
    target_sources(pegasus-backend PRIVATE
        model/internal/GamepadManagerSDL2.cpp
        model/internal/GamepadManagerSDL2.h
    )
    target_compile_definitions(pegasus-backend PRIVATE WITH_SDL_GAMEPAD)
    set(USE_SDL2 ON)
    set(GAMEPAD_IMPL_NAME "SDL2")
else()
    target_sources(pegasus-backend PRIVATE
        model/internal/GamepadManagerQt.cpp
        model/internal/GamepadManagerQt.h
    )
    pegasus_require_qt(COMPONENTS Gamepad)
    target_link_libraries(pegasus-backend PUBLIC Qt::Gamepad)
    set(GAMEPAD_IMPL_NAME "Qt")
endif()


# Select battery info implementation

if(PEGASUS_USE_SDL2_POWER)
    target_compile_definitions(pegasus-backend PRIVATE WITH_SDL_POWER)
    set(BATTERY_IMPL_NAME "SDL2")
    set(USE_SDL2 ON)
elseif(PEGASUS_ON_ANDROID)
    set(BATTERY_IMPL_NAME "Android")
else()
    set(BATTERY_IMPL_NAME "Disabled")
endif()

# Animated PNG

if(PEGASUS_ENABLE_APNG)
    target_link_libraries(pegasus-backend PUBLIC apng)
endif()


# SDL2

if(USE_SDL2)
    find_package(SDL2 REQUIRED)
    target_include_directories(pegasus-backend PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(pegasus-backend PUBLIC ${SDL2_LIBRARIES})
    target_compile_definitions(pegasus-backend PRIVATE SDL_MAIN_HANDLED)
endif()


# Print config

include(PegasusPrettyPrint)

set(BOX_LINES
    "Power management: ${POWER_IMPL_NAME}"
    "Battery info: ${BATTERY_IMPL_NAME}"
    "Gamepad implementation: ${GAMEPAD_IMPL_NAME}"
    "Animated PNG support: ${PEGASUS_ENABLE_APNG}"
    "Third-party data sources:"
)
list(SORT OPTIONAL_PROVIDERS)
foreach(PROVIDER IN LISTS OPTIONAL_PROVIDERS)
    list(APPEND BOX_LINES "  - ${PROVIDER}")
endforeach()

pegasus_print_box(
    TITLE "Backend Settings"
    CONTENT "${BOX_LINES}"
)
