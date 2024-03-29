function(create_platform_var_maybe varsuffix sysname)
    if("${CMAKE_SYSTEM_NAME}" MATCHES "${sysname}")
        set("PEGASUS_ON_${varsuffix}" ON CACHE INTERNAL "")
    endif()
endfunction()


create_platform_var_maybe("ANDROID" "Android")
create_platform_var_maybe("WINDOWS" "Windows")
create_platform_var_maybe("MACOS" "Darwin")
if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm|aarch64")
        set(PEGASUS_ON_EGLFS ON)
    else()
        set(PEGASUS_ON_X11 ON)
    endif()
endif()
