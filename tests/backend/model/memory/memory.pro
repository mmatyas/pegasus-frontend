QT += qml testlib
CONFIG += c++11 warn_on testcase no_testcase_installs

TARGET = test_Memory
SOURCES = test_Memory.cpp Container.cpp
HEADERS = Container.h
DEFINES *= $${COMMON_DEFINES}

include($${TOP_SRCDIR}/src/link_to_backend.pri)
