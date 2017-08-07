CONFIG += testcase
TARGET = test_Utils

QT += testlib
CONFIG += c++11 warn_on exceptions_off


include($${TOP_SRCDIR}/src/link_to_backend.pri)
