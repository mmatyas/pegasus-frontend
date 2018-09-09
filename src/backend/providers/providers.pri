HEADERS += \
    $$PWD/Provider.h \
    $$PWD/ProviderManager.h \
    $$PWD/pegasus/PegasusCollections.h \
    $$PWD/pegasus/PegasusCommon.h \
    $$PWD/pegasus/PegasusMetadata.h \
    $$PWD/pegasus/PegasusProvider.h \
    $$PWD/pegasus_favorites/Favorites.h \
    $$PWD/pegasus_playtime/PlaytimeStats.h

SOURCES += \
    $$PWD/Provider.cpp \
    $$PWD/ProviderManager.cpp \
    $$PWD/pegasus/PegasusCollections.cpp \
    $$PWD/pegasus/PegasusCommon.cpp \
    $$PWD/pegasus/PegasusMetadata.cpp \
    $$PWD/pegasus/PegasusProvider.cpp \
    $$PWD/pegasus_favorites/Favorites.cpp \
    $$PWD/pegasus_playtime/PlaytimeStats.cpp


contains(QMAKE_CXX, ".*arm.*")|contains(QMAKE_CXX, ".*aarch.*"): target_arm = yes
unix:!macx:!android:!defined(target_arm, var): pclinux = yes
unix:!android:defined(target_arm, var): armlinux = yes


ENABLED_COMPATS =

win32|macx|defined(pclinux,var) {
    ENABLED_COMPATS += Steam
    DEFINES *= WITH_COMPAT_STEAM
    HEADERS += \
        $$PWD/steam/SteamGamelist.h \
        $$PWD/steam/SteamMetadata.h \
        $$PWD/steam/SteamProvider.h
    SOURCES += \
        $$PWD/steam/SteamGamelist.cpp \
        $$PWD/steam/SteamMetadata.cpp \
        $$PWD/steam/SteamProvider.cpp
}

win32|defined(pclinux,var) {
    ENABLED_COMPATS += GOG
    DEFINES *= WITH_COMPAT_GOG
    HEADERS += \
        $$PWD/gog/GogCommon.h \
        $$PWD/gog/GogGamelist.h \
        $$PWD/gog/GogMetadata.h \
        $$PWD/gog/GogProvider.h
    SOURCES += \
        $$PWD/gog/GogCommon.cpp \
        $$PWD/gog/GogGamelist.cpp \
        $$PWD/gog/GogMetadata.cpp \
        $$PWD/gog/GogProvider.cpp
}

win32|macx|defined(pclinux,var)|defined(armlinux,var) {
    ENABLED_COMPATS += EmulationStation
    DEFINES *= WITH_COMPAT_ES2
    HEADERS += \
        $$PWD/es2/Es2Metadata.h \
        $$PWD/es2/Es2Provider.h \
        $$PWD/es2/Es2Systems.h \

    SOURCES += \
        $$PWD/es2/Es2Metadata.cpp \
        $$PWD/es2/Es2Provider.cpp \
        $$PWD/es2/Es2Systems.cpp \
}


contains(ENABLED_COMPATS,Steam)|contains(ENABLED_COMPATS,GOG) {
    HEADERS += \
        $$PWD/JsonCacheUtils.h
    SOURCES += \
        $$PWD/JsonCacheUtils.cpp
}


# Print configuration
ENABLED_COMPATS = $$sorted(ENABLED_COMPATS)
message("Enabled third-party data sources:")
for(name, ENABLED_COMPATS): message("  - $$name")
isEmpty(ENABLED_COMPATS): message("  - (none)")
