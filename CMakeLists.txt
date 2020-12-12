cmake_minimum_required(VERSION 3.9.0)

project(pegasus-fe)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_SKIP_RPATH ON)

# Enable processing Qt generated files
if (POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()


option(PEGASUS_USE_SDL2_GAMEPAD "Use SDL2 for gamepad support" ON)


execute_process(COMMAND
    git
    --git-dir "${PROJECT_SOURCE_DIR}/.git"
    --work-tree "${PROJECT_SOURCE_DIR}"
    describe
    --always
    --dirty
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE PEGASUS_GIT_REVISION
)
execute_process(COMMAND
    git
    --git-dir "${PROJECT_SOURCE_DIR}/.git"
    --work-tree "${PROJECT_SOURCE_DIR}"
    log -1
    --format=%cd
    --date=short
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE PEGASUS_GIT_DATE
)
execute_process(COMMAND
    git
    --git-dir "${PROJECT_SOURCE_DIR}/.git"
    --work-tree "${PROJECT_SOURCE_DIR}"
    rev-list
    --count HEAD
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE PEGASUS_GIT_COMMIT_CNT
)


add_subdirectory(thirdparty/QtQmlTricks)
add_subdirectory(thirdparty/SortFilterProxyModel)
add_subdirectory(assets)
add_subdirectory(src)


enable_testing(true)
add_subdirectory(tests)
