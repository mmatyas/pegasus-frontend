CONFIG += testcase
TARGET = test_Api

QT += qml testlib
CONFIG += c++11 warn_on exceptions_off

SOURCES = test_Api.cpp

RESOURCES += \
    $${TOP_SRCDIR}/src/themes/themes.qrc


include($${TOP_SRCDIR}/src/link_to_backend.pri)
