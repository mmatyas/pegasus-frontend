include(PegasusCommonProps)
include(PegasusTargetPlatform)

include(PegasusQtUtils)
pegasus_require_qt(COMPONENTS
    Test
    QuickTest
    Qml
    QmlImportScanner
)


function(pegasus_cxx_test name)
    add_executable("${name}" "${name}.cpp")
    add_test(NAME "${name}" COMMAND "${name}")

    target_link_libraries("${name}" PRIVATE Qt::Test pegasus-backend)
    pegasus_add_common_props("${name}")
endfunction()


function(pegasus_qml_test name)
    add_executable("${name}" "${name}.cpp")
    add_test(NAME "${name}" COMMAND "${name}" -input "${CMAKE_CURRENT_LIST_DIR}")
    qt_import_qml_plugins("${name}")

    target_link_libraries("${name}" PRIVATE Qt::QuickTest pegasus-backend)
    pegasus_add_common_props("${name}")
endfunction()


add_subdirectory(backend/api)
add_subdirectory(backend/configfile)
add_subdirectory(backend/model/collection)
add_subdirectory(backend/model/game)
add_subdirectory(backend/model/gameassets)
add_subdirectory(backend/model/keyeditor)
add_subdirectory(backend/model/locales)
add_subdirectory(backend/model/memory)
add_subdirectory(backend/model/system)
add_subdirectory(backend/model/themes)
add_subdirectory(backend/processlauncher)
add_subdirectory(backend/providers/favorites)
add_subdirectory(backend/providers/logiqx)
add_subdirectory(backend/providers/pegasus)
add_subdirectory(backend/providers/pegasus_media)
add_subdirectory(backend/providers/playtime)
add_subdirectory(backend/utils)

if(PEGASUS_ON_WINDOWS OR PEGASUS_ON_MACOS OR PEGASUS_ON_X11 OR PEGASUS_ON_EGLFS)
    add_subdirectory(backend/providers/emulationstation)
endif()
if(PEGASUS_ON_WINDOWS)
    add_subdirectory(backend/providers/launchbox)
    add_subdirectory(backend/providers/playnite)
endif()

add_subdirectory(integration/blurhash)
add_subdirectory(integration/sortfilter)

if(PEGASUS_USE_SDL2_GAMEPAD)
    add_subdirectory(integration/sdl_gamepad)
endif()

if(PEGASUS_ENABLE_APNG)
    add_subdirectory(integration/apng)
endif()

add_subdirectory(benchmarks/configfile)
add_subdirectory(benchmarks/pegasus_provider)
