ENABLED_COMPATS += LaunchBox

DEFINES *= WITH_COMPAT_LAUNCHBOX

HEADERS += \
    $$PWD/LaunchBoxAssets.h \
    $$PWD/LaunchBoxEmulator.h \
    $$PWD/LaunchBoxEmulatorsXml.h \
    $$PWD/LaunchBoxGamelistXml.h \
    $$PWD/LaunchBoxPlatformsXml.h \
    $$PWD/LaunchBoxProvider.h \
    $$PWD/LaunchBoxXml.h

SOURCES += \
    $$PWD/LaunchBoxAssets.cpp \
    $$PWD/LaunchBoxEmulatorsXml.cpp \
    $$PWD/LaunchBoxGamelistXml.cpp \
    $$PWD/LaunchBoxPlatformsXml.cpp \
    $$PWD/LaunchBoxProvider.cpp \
    $$PWD/LaunchBoxXml.cpp
