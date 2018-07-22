CONFIG += testcase no_testcase_installs

QT += qml testlib
CONFIG += c++11 warn_on exceptions_off

TARGET = test_Playtime
SOURCES = $${TARGET}.cpp
DEFINES *= $${COMMON_DEFINES}
RESOURCES += data.qrc

include($${TOP_SRCDIR}/src/link_to_backend.pri)
