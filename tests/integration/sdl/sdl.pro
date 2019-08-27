CONFIG += testcase no_testcase_installs warn_on
QT += testlib

TARGET = test_SDL
SOURCES = $${TARGET}.cpp
DEFINES *= $${COMMON_DEFINES}

include($${TOP_SRCDIR}/src/link_to_backend.pri)
