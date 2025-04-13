find_package(Qt6 REQUIRED COMPONENTS
    Qml
    Quick
)

qt_add_resources(PEGASUS_QML_ASSETS
    frontend.qrc
    "${PROJECT_SOURCE_DIR}/src/qmlutils/qmlutils.qrc"  # FIXME
    "${PROJECT_SOURCE_DIR}/src/themes/themes.qrc"  # FIXME
)

add_library(pegasus-qml "${PEGASUS_QML_ASSETS}")
target_link_libraries(pegasus-qml PUBLIC
    Qt::Qml
    Qt::Quick
)


include(PegasusCommonProps)
pegasus_add_common_props_optimized(pegasus-qml)
