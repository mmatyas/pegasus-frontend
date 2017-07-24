lessThan(QT_MAJOR_VERSION, 5) | lessThan(QT_MINOR_VERSION, 7) {
    message("Cannot build this project using Qt $$[QT_VERSION]")
    error("This project requires at least Qt 5.7 or newer")
}
lessThan(QT_MINOR_VERSION, 9) {
    warning("Qt 5.9 is recommended for the best performance")
}


TEMPLATE = subdirs

SUBDIRS += \
    src

OTHER_FILES += \
    assets/*


include($${TOP_SRCDIR}/src/print_config.pri)
