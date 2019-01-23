HEADERS += \
    $$PWD/Provider.h \
    $$PWD/ProviderManager.h \
    $$PWD/EnabledProviders.h

SOURCES += \
    $$PWD/Provider.cpp \
    $$PWD/ProviderManager.cpp \

include(pegasus/pegasus.pri)
include(pegasus_favorites/favorites.pri)
include(pegasus_playtime/playtime.pri)

contains(QMAKE_CXX, ".*arm.*")|contains(QMAKE_CXX, ".*aarch.*"): target_arm = yes
unix:!macx:!android:!defined(target_arm, var): pclinux = yes
unix:!android:defined(target_arm, var): armlinux = yes


ENABLED_COMPATS =

win32|macx|defined(pclinux,var): include(steam/steam.pri)
win32|defined(pclinux,var): include(gog/gog.pri)
win32|macx|defined(pclinux,var)|defined(armlinux,var): include(es2/es2.pri)
android: include(android_apps/android.pri)

# # All
# {
#     ENABLED_COMPATS += "Skraper Assets"
#     DEFINES *= WITH_COMPAT_SKRAPER
#     HEADERS += $$PWD/skraper/SkraperAssetsProvider.h
#     SOURCES += $$PWD/skraper/SkraperAssetsProvider.cpp
# }


defined(USES_JSON_CACHE, var) {
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
