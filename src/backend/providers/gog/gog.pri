ENABLED_COMPATS += GOG
USES_JSON_CACHE = yes

DEFINES *= WITH_COMPAT_GOG

HEADERS += \
    $$PWD/GogGamelist.h \
    $$PWD/GogMetadata.h \
    $$PWD/GogProvider.h \

SOURCES += \
    $$PWD/GogGamelist.cpp \
    $$PWD/GogMetadata.cpp \
    $$PWD/GogProvider.cpp \
