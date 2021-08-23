cmake_minimum_required(VERSION 3.16)

project(pegasus-frontend
    DESCRIPTION "Cross platform, customizable graphical frontend for launching games"
    HOMEPAGE_URL "https://pegasus-frontend.org/"
    LANGUAGES CXX
)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/cmake")


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_SKIP_RPATH ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

cmake_policy(SET CMP0071 NEW)  # Enable processing Qt generated files


option(PEGASUS_USE_SDL2_GAMEPAD "Use SDL2 for gamepad support" ON)
option(PEGASUS_USE_SDL2_POWER "Use SDL2 for battery info support" ON)
option(PEGASUS_ENABLE_APNG "Enable animated PNG support" OFF)
option(PEGASUS_STATIC_CXX "Link stdc++ statically" OFF)
option(PEGASUS_ENABLE_LTO "Allow link-time optimizations (slow to build)" ON)


include(PegasusGitInfo)


if(PEGASUS_ENABLE_APNG)
    add_subdirectory(thirdparty/apng)
endif()
add_subdirectory(thirdparty/QtQmlTricks)
add_subdirectory(thirdparty/SortFilterProxyModel)
add_subdirectory(assets)
add_subdirectory(src)


enable_testing(true)
add_subdirectory(tests)
