find_package(Qt5 REQUIRED COMPONENTS Gui)

add_library(apng
    apngimagehandler.cpp
    apngimagehandler_p.h
    apngimageplugin.cpp
    apngimageplugin.h
    apngreader.cpp
    apngreader_p.h
)
target_link_libraries(apng PUBLIC Qt5::Gui)

if(PEGASUS_STATIC)
    target_compile_definitions(apng PRIVATE QT_STATICPLUGIN)
endif()
