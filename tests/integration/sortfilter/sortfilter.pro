TARGET = test_SortFilter
SOURCES = $${TARGET}.cpp

OTHER_FILES += \
    tst_all.qml

include($${TOP_SRCDIR}/tests/qmltest_common.pri)
