TEMPLATE = subdirs

SUBDIRS += \
    pegasus \
    pegasus_media \
    emulationstation \
    favorites \
    logiqx \
    playtime \

win32: SUBDIRS += \
    launchbox \
    playnite \
