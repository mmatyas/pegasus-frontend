lessThan(QT_MAJOR_VERSION, 5) | lessThan(QT_MINOR_VERSION, 8) {
    message("Cannot build this project using Qt $$[QT_VERSION]")
    error("This project requires at least Qt 5.8 or newer")
}
lessThan(QT_MINOR_VERSION, 9) {
    warning("Qt 5.9 or later is recommended for the best performance")
}


TEMPLATE = subdirs
SUBDIRS += src

# FIXME: MAke sure the QT options of Backend inherit to the dependers
qtHaveModule(testlib):!android {
    SUBDIRS += tests
    tests.depends = src
    tests.CONFIG = no_default_install
}


include($${TOP_SRCDIR}/src/deployment_vars.pri)
include($${TOP_SRCDIR}/src/print_config.pri)
