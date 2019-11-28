TARGET = test_KeyEditor
SOURCES = $${TARGET}.cpp

OTHER_FILES += \
    tst_add.qml \
    tst_delete.qml \
    tst_keyname.qml \
    tst_replace.qml \
    utils.js

include($${TOP_SRCDIR}/tests/qmltest_common.pri)
