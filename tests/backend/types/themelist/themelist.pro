CONFIG += testcase

QT += qml testlib
CONFIG += c++11 warn_on exceptions_off

TARGET = test_ThemeList
SOURCES = $${TARGET}.cpp
DEFINES *= $${COMMON_DEFINES}

RESOURCES = $${TOP_SRCDIR}/src/themes/themes.qrc

include($${TOP_SRCDIR}/src/link_to_backend.pri)
