TARGET = test_Themes
SOURCES = $${TARGET}.cpp

RESOURCES = $${TOP_SRCDIR}/src/themes/themes.qrc

include($${TOP_SRCDIR}/tests/cxxtest_common.pri)
