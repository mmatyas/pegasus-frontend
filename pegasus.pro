REQ_QT_MAJOR = 5
REQ_QT_MINOR = 12

lessThan(QT_MAJOR_VERSION, $${REQ_QT_MAJOR}) | equals(QT_MAJOR_VERSION, $${REQ_QT_MAJOR}):lessThan(QT_MINOR_VERSION, $${REQ_QT_MINOR}) {
    message("Cannot build this project using Qt $$[QT_VERSION]")
    error("This project requires at least Qt $${REQ_QT_MAJOR}.$${REQ_QT_MINOR} or newer")
}


TEMPLATE = subdirs
SUBDIRS += src thirdparty
OTHER_FILES += .qmake.conf

src.depends = thirdparty

# FIXME: MAke sure the QT options of Backend inherit to the dependers
qtHaveModule(testlib):!android {
    SUBDIRS += tests
    tests.depends = src
    tests.CONFIG = no_default_install
}


include($${TOP_SRCDIR}/src/deployment_vars.pri)
include($${TOP_SRCDIR}/src/print_config.pri)
