CONFIG += testcase no_testcase_installs

QT += qml testlib
CONFIG += c++11 warn_on exceptions_off

TARGET = test_ProcessLauncher
SOURCES = $${TARGET}.cpp
DEFINES *= $${COMMON_DEFINES}

include($${TOP_SRCDIR}/src/link_to_backend.pri)
