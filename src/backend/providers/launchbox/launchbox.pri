ENABLED_COMPATS += LaunchBox

DEFINES *= WITH_COMPAT_LAUNCHBOX

HEADERS += \
    $$PWD/LaunchBoxCommon.h \
    $$PWD/LaunchBoxEmulatorsXml.h \
    $$PWD/LaunchBoxGamelistXml.h \
    $$PWD/LaunchBoxPlatformsXml.h \
    $$PWD/LaunchBoxProvider.h

SOURCES += \
    $$PWD/LaunchBoxCommon.cpp \
    $$PWD/LaunchBoxEmulatorsXml.cpp \
    $$PWD/LaunchBoxGamelistXml.cpp \
    $$PWD/LaunchBoxPlatformsXml.cpp \
    $$PWD/LaunchBoxProvider.cpp
