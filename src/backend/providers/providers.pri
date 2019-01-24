HEADERS += \
    $$PWD/Provider.h \
    $$PWD/ProviderManager.h \
    #$$PWD/pegasus_playtime/PlaytimeStats.h \
    $$PWD/EnabledProviders.h

SOURCES += \
    $$PWD/Provider.cpp \
    $$PWD/ProviderManager.cpp \
    #$$PWD/pegasus_playtime/PlaytimeStats.cpp \

include(pegasus/pegasus.pri)
include(pegasus_favorites/favorites.pri)

contains(QMAKE_CXX, ".*arm.*")|contains(QMAKE_CXX, ".*aarch.*"): target_arm = yes
unix:!macx:!android:!defined(target_arm, var): pclinux = yes
unix:!android:defined(target_arm, var): armlinux = yes


ENABLED_COMPATS =

# win32|macx|defined(pclinux,var) {
#     ENABLED_COMPATS += Steam
#     uses_json_cache = yes
#     DEFINES *= WITH_COMPAT_STEAM
#     HEADERS += \
#         $$PWD/steam/SteamGamelist.h \
#         $$PWD/steam/SteamMetadata.h \
#         $$PWD/steam/SteamProvider.h
#     SOURCES += \
#         $$PWD/steam/SteamGamelist.cpp \
#         $$PWD/steam/SteamMetadata.cpp \
#         $$PWD/steam/SteamProvider.cpp
# }
#.
# win32|defined(pclinux,var) {
#     ENABLED_COMPATS += GOG
#     uses_json_cache = yes
#     DEFINES *= WITH_COMPAT_GOG
#     HEADERS += \
#         $$PWD/gog/GogCommon.h \
#         $$PWD/gog/GogGamelist.h \
#         $$PWD/gog/GogMetadata.h \
#         $$PWD/gog/GogProvider.h
#     SOURCES += \
#         $$PWD/gog/GogCommon.cpp \
#         $$PWD/gog/GogGamelist.cpp \
#         $$PWD/gog/GogMetadata.cpp \
#         $$PWD/gog/GogProvider.cpp
# }
#.
# win32|macx|defined(pclinux,var)|defined(armlinux,var) {
#     ENABLED_COMPATS += EmulationStation
#     DEFINES *= WITH_COMPAT_ES2
#     HEADERS += \
#         $$PWD/es2/Es2Metadata.h \
#         $$PWD/es2/Es2Provider.h \
#         $$PWD/es2/Es2Systems.h \
#
#     SOURCES += \
#         $$PWD/es2/Es2Metadata.cpp \
#         $$PWD/es2/Es2Provider.cpp \
#         $$PWD/es2/Es2Systems.cpp \
# }
#.
# android {
#     ENABLED_COMPATS *= "Android Apps"
#     uses_json_cache = yes
#     DEFINES *= WITH_COMPAT_ANDROIDAPPS
#     HEADERS += \
#         $$PWD/android_apps/AndroidAppsProvider.h \
#         $$PWD/android_apps/AndroidAppsMetadata.h
#     SOURCES += \
#         $$PWD/android_apps/AndroidAppsProvider.cpp \
#         $$PWD/android_apps/AndroidAppsMetadata.cpp
# }
#.
# # All
# {
#     ENABLED_COMPATS += "Skraper Assets"
#     DEFINES *= WITH_COMPAT_SKRAPER
#     HEADERS += $$PWD/skraper/SkraperAssetsProvider.h
#     SOURCES += $$PWD/skraper/SkraperAssetsProvider.cpp
# }


defined(uses_json_cache,var) {
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
