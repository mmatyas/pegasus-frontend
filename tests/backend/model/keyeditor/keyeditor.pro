TEMPLATE = app
QT += qml
CONFIG += c++11 warn_on qmltestcase no_testcase_installs

TARGET = test_KeyEditor
SOURCES = $${TARGET}.cpp
DEFINES *= $${COMMON_DEFINES}

include($${TOP_SRCDIR}/src/link_to_backend.pri)

OTHER_FILES += \
    tst_add.qml \
    tst_delete.qml \
    tst_keyname.qml \
    tst_replace.qml \
    utils.js
