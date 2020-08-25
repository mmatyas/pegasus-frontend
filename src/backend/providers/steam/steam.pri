ENABLED_COMPATS += Steam
USES_JSON_CACHE = yes

DEFINES *= WITH_COMPAT_STEAM

HEADERS += \
    $$PWD/SteamCommon.h \
    $$PWD/SteamGamelist.h \
    $$PWD/SteamMetadata.h \
    $$PWD/SteamProvider.h \

SOURCES += \
    $$PWD/SteamCommon.cpp \
    $$PWD/SteamGamelist.cpp \
    $$PWD/SteamMetadata.cpp \
    $$PWD/SteamProvider.cpp \
