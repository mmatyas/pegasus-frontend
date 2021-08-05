include(CheckIPOSupported)
include(CheckCXXCompilerFlag)


function(pegasus_add_common_props target)
    target_compile_definitions(${target} PRIVATE
        QT_DEPRECATED_WARNINGS
        QT_DISABLE_DEPRECATED_BEFORE=0x050F00
        QT_NO_CAST_TO_ASCII
        QT_NO_FOREACH
        QT_NO_JAVA_STYLE_ITERATORS
        QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
        QT_NO_PROCESS_COMBINED_ARGUMENT_START
        QT_NO_URL_CAST_FROM_STRING
        GIT_REVISION="${PEGASUS_GIT_REVISION}"
        GIT_DATE="${PEGASUS_GIT_DATE}"
    )

    target_compile_features(${target} PUBLIC cxx_std_17)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    target_link_options(${target} PRIVATE -fno-exceptions)

    if(MSVC)
        target_compile_options(${target} PRIVATE /W3)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -pedantic)
    endif()
endfunction()


function(pegasus_add_common_props_optimized target)
    pegasus_add_common_props(${target})

    check_ipo_supported(RESULT ipo_supported)
    if(ipo_supported)
        set_target_properties(${target} PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
    endif()

    if(PEGASUS_STATIC_CXX)
        if(NOT MSVC)
            target_link_options(${target} PRIVATE -static-libstdc++)
        endif()
    endif()
endfunction()
