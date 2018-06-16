CONFIG += testcase no_testcase_installs

QT += qml testlib
CONFIG += c++11 warn_on exceptions_off

TARGET = test_LocaleList
SOURCES = $${TARGET}.cpp
DEFINES *= $${COMMON_DEFINES}

#RESOURCES = $${TOP_BUILDDIR}/src/app/lang/translations.qrc

include($${TOP_SRCDIR}/src/link_to_backend.pri)
