HEADERS += \
    $$PWD/Provider.h \
    $$PWD/ProviderManager.h \
    $$PWD/ProviderUtils.h \
    $$PWD/SearchContext.h \

SOURCES += \
    $$PWD/Provider.cpp \
    $$PWD/ProviderManager.cpp \
    $$PWD/ProviderUtils.cpp \
    $$PWD/SearchContext.cpp \

include(pegasus_favorites/pegasus_favorites.pri)
include(pegasus_metadata/pegasus_metadata.pri)
include(pegasus_media/pegasus_media.pri)
include(pegasus_playtime/pegasus_playtime.pri)

contains(QMAKE_CXX, ".*arm.*")|contains(QMAKE_CXX, ".*aarch.*"): target_arm = yes
unix:!macx:!android:!defined(target_arm, var): pclinux = yes
unix:!android:defined(target_arm, var): armlinux = yes


ENABLED_COMPATS =

win32|macx|defined(pclinux,var): include(steam/steam.pri)
win32|defined(pclinux,var): include(gog/gog.pri)
win32|macx|defined(pclinux,var)|defined(armlinux,var): include(es2/es2.pri)
win32: include(launchbox/launchbox.pri)
win32: include(playnite/playnite.pri)
win32: include(epicgames/epicgames.pri)
android: include(android_apps/android.pri)
defined(pclinux,var): include(lutris/lutris.pri)
# All platforms
include(logiqx/logiqx.pri)
include(skraper/skraper.pri)


defined(USES_JSON_CACHE, var) {
    DEFINES *= WITH_JSON_CACHE
}


# Print configuration
ENABLED_COMPATS = $$sorted(ENABLED_COMPATS)
message("Enabled third-party data sources:")
for(name, ENABLED_COMPATS): message("  - $$name")
isEmpty(ENABLED_COMPATS): message("  - (none)")
