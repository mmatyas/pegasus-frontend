CONFIG += testcase no_testcase_installs

CONFIG += qmltestcase

TARGET = test_SortFilter
SOURCES = $${TARGET}.cpp
DEFINES *= $${COMMON_DEFINES}

include($${TOP_SRCDIR}/src/link_to_backend.pri)

DISTFILES += \
    tst_all.qml
