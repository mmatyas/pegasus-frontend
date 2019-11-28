CONFIG += testcase no_testcase_installs
CONFIG += c++11 warn_on exceptions_off rtti_off

QT += qml testlib

DEFINES *= $${COMMON_DEFINES}

include($${TOP_SRCDIR}/src/link_to_backend.pri)
