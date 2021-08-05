find_package(Qt6 COMPONENTS Gui QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Gui REQUIRED)
endif()


add_library(apng
    apngimagehandler.cpp
    apngimagehandler_p.h
    apngimageplugin.cpp
    apngimageplugin.h
    apngreader.cpp
    apngreader_p.h
)

target_link_libraries(apng PUBLIC Qt::Gui)
target_compile_definitions(apng PRIVATE QT_STATICPLUGIN)
