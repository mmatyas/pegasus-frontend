# NOTE: This is a macro because `find_package` variables would go
# out of scope at the end of functions
macro(pegasus_require_qt)
    cmake_parse_arguments(QTREQ
        ""
        ""
        COMPONENTS
        ${ARGN})

    find_package(Qt6 COMPONENTS ${QTREQ_COMPONENTS} QUIET)
    if (NOT Qt6_FOUND)
        find_package(Qt5 5.15 COMPONENTS ${QTREQ_COMPONENTS} REQUIRED)
    endif()
endmacro()
