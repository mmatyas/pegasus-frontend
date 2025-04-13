include(PegasusTargetPlatform)
include(PegasusQtUtils)

set(USE_SDL2 OFF CACHE INTERNAL "")


add_library(pegasus-backend
    AppSettings.cpp
    AppSettings.h
    Backend.cpp
    Backend.h
    CliArgs.h
    FrontendLayer.cpp
    FrontendLayer.h
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
)

add_subdirectory(parsers)
add_subdirectory(model)
add_subdirectory(platform)
add_subdirectory(providers)
add_subdirectory(types)
add_subdirectory(utils)

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
    SortFilterProxyModel
)

include(PegasusCommonProps)
pegasus_add_common_props_optimized(pegasus-backend)


# Animated PNG

if(PEGASUS_ENABLE_APNG)
    target_link_libraries(pegasus-backend PUBLIC apng)
endif()


# SDL2

if(USE_SDL2)
    find_package(SDL2 REQUIRED)
    target_include_directories(pegasus-backend PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(pegasus-backend PUBLIC ${SDL2_LIBRARIES})
    target_compile_definitions(pegasus-backend PUBLIC SDL_MAIN_HANDLED)
    message("-- Found SDL2: ${SDL2_INCLUDE_DIRS}")
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
