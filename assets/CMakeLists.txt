include(PegasusQtUtils)
pegasus_require_qt(COMPONENTS Core)

qt_add_big_resources(PEGASUS_ASSETS_SRC assets.qrc)
add_library(pegasus-assets ${PEGASUS_ASSETS_SRC})

include(PegasusCommonProps)
pegasus_add_common_props(pegasus-assets)
