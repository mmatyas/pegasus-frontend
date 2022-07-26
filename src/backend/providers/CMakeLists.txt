target_sources(pegasus-backend PRIVATE
    Provider.cpp
    Provider.h
    ProviderManager.cpp
    ProviderManager.h
    ProviderUtils.cpp
    ProviderUtils.h
    SearchContext.cpp
    SearchContext.h
)


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


add_subdirectory(android_apps)
add_subdirectory(epic)
add_subdirectory(es2)
add_subdirectory(gog)
add_subdirectory(launchbox)
add_subdirectory(logiqx)
add_subdirectory(lutris)
add_subdirectory(pegasus_favorites)
add_subdirectory(pegasus_media)
add_subdirectory(pegasus_metadata)
add_subdirectory(pegasus_playtime)
add_subdirectory(playnite)
add_subdirectory(skraper)
add_subdirectory(steam)


if(USE_JSON_CACHE)
    target_compile_definitions(pegasus-backend PRIVATE WITH_JSON_CACHE)
endif()
