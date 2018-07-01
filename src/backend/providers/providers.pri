HEADERS += \
    providers/Provider.h \
    providers/pegasus/PegasusCollections.h \
    providers/pegasus/PegasusCommon.h \
    providers/pegasus/PegasusMetadata.h \
    providers/pegasus/PegasusProvider.h \

SOURCES += \
    providers/Provider.cpp \
    providers/pegasus/PegasusCollections.cpp \
    providers/pegasus/PegasusCommon.cpp \
    providers/pegasus/PegasusMetadata.cpp \
    providers/pegasus/PegasusProvider.cpp \

!android:!contains(QMAKE_CXX, ".*arm.*"):!contains(QMAKE_CXX, ".*aarch.*") {
    DEFINES *= WITH_COMPAT_STEAM
    HEADERS += \
        providers/steam/SteamGamelist.h \
        providers/steam/SteamMetadata.h \
        providers/steam/SteamProvider.h \

    SOURCES += \
        providers/steam/SteamGamelist.cpp \
        providers/steam/SteamMetadata.cpp \
        providers/steam/SteamProvider.cpp \
}
!android {
    DEFINES *= WITH_COMPAT_ES2
    HEADERS += \
        providers/es2/Es2Metadata.h \
        providers/es2/Es2Provider.h \
        providers/es2/Es2Systems.h \

    SOURCES += \
        providers/es2/Es2Metadata.cpp \
        providers/es2/Es2Provider.cpp \
        providers/es2/Es2Systems.cpp \
}
