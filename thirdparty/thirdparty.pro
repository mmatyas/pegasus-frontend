TEMPLATE = subdirs

!isEmpty(ENABLE_APNG): {
    SUBDIRS += apng
}

OTHER_FILES += link_to_png.pri
