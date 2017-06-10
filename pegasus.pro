lessThan(QT_MAJOR_VERSION, 5) | lessThan(QT_MINOR_VERSION, 8) {
    message("Cannot build this project using Qt $$[QT_VERSION]")
    error("This project requires at least Qt 5.8 or newer")
}
lessThan(QT_MINOR_VERSION, 9) {
    warning("Qt 5.8 support will be dropped in the next release")
    warning("Please update to 5.9 or newer")
}

TEMPLATE = subdirs

SUBDIRS += \
    src

OTHER_FILES += \
    assets/*


message("Git revision: '$${GIT_REVISION}'")
