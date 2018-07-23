HEADERS += \
    $$PWD/Provider.h \
    $$PWD/ProviderManager.h \
    $$PWD/pegasus/PegasusCollections.h \
    $$PWD/pegasus/PegasusCommon.h \
    $$PWD/pegasus/PegasusMetadata.h \
    $$PWD/pegasus/PegasusProvider.h \
    $$PWD/pegasus_favorites/Favorites.h \
    $$PWD/pegasus_playtime/PlaytimeStats.h \

SOURCES += \
    $$PWD/Provider.cpp \
    $$PWD/ProviderManager.cpp \
    $$PWD/pegasus/PegasusCollections.cpp \
    $$PWD/pegasus/PegasusCommon.cpp \
    $$PWD/pegasus/PegasusMetadata.cpp \
    $$PWD/pegasus/PegasusProvider.cpp \
    $$PWD/pegasus_favorites/Favorites.cpp \
    $$PWD/pegasus_playtime/PlaytimeStats.cpp \

!android:!contains(QMAKE_CXX, ".*arm.*"):!contains(QMAKE_CXX, ".*aarch.*") {
    DEFINES *= WITH_COMPAT_STEAM
    HEADERS += \
        $$PWD/steam/SteamGamelist.h \
        $$PWD/steam/SteamMetadata.h \
        $$PWD/steam/SteamProvider.h \

    SOURCES += \
        $$PWD/steam/SteamGamelist.cpp \
        $$PWD/steam/SteamMetadata.cpp \
        $$PWD/steam/SteamProvider.cpp \
}
!android {
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
