ENABLED_COMPATS += EmulationStation

DEFINES *= WITH_COMPAT_ES2

HEADERS += \
    $$PWD/Es2Metadata.h \
    $$PWD/Es2Provider.h \
    $$PWD/Es2Systems.h \

SOURCES += \
    $$PWD/Es2Metadata.cpp \
    $$PWD/Es2Provider.cpp \
    $$PWD/Es2Systems.cpp \
