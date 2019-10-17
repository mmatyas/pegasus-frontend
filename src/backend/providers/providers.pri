HEADERS += \
    $$PWD/Provider.h \
    $$PWD/ProviderManager.h \

SOURCES += \
    $$PWD/Provider.cpp \
    $$PWD/ProviderManager.cpp \

include(pegasus_favorites/pegasus_favorites.pri)
include(pegasus_metadata/pegasus_metadata.pri)
include(pegasus_playtime/pegasus_playtime.pri)

contains(QMAKE_CXX, ".*arm.*")|contains(QMAKE_CXX, ".*aarch.*"): target_arm = yes
unix:!macx:!android:!defined(target_arm, var): pclinux = yes
unix:!android:defined(target_arm, var): armlinux = yes


ENABLED_COMPATS =

win32|macx|defined(pclinux,var): include(steam/steam.pri)
win32|defined(pclinux,var): include(gog/gog.pri)
win32|macx|defined(pclinux,var)|defined(armlinux,var): include(es2/es2.pri)
win32: include(launchbox/launchbox.pri)
android: include(android_apps/android.pri)
defined(pclinux,var): include(lutris/lutris.pri)
# All platforms
include(skraper/skraper.pri)


defined(USES_JSON_CACHE, var) {
    HEADERS += $$PWD/JsonCacheUtils.h
    SOURCES += $$PWD/JsonCacheUtils.cpp
}


# Print configuration
ENABLED_COMPATS = $$sorted(ENABLED_COMPATS)
message("Enabled third-party data sources:")
for(name, ENABLED_COMPATS): message("  - $$name")
isEmpty(ENABLED_COMPATS): message("  - (none)")
