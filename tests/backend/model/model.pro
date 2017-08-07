CONFIG += testcase
TARGET = test_Model

QT += qml testlib
CONFIG += c++11 warn_on exceptions_off

SOURCES = test_Model.cpp


include($${TOP_SRCDIR}/src/link_to_backend.pri)
