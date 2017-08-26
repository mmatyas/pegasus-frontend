CONFIG += testcase
TARGET = test_Utils

QT += testlib
CONFIG += c++11 warn_on exceptions_off

SOURCES = test_Utils.cpp
DEFINES *= $${COMMON_DEFINES}


include($${TOP_SRCDIR}/src/link_to_backend.pri)
