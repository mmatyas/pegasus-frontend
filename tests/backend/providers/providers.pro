TEMPLATE = subdirs

SUBDIRS += \
    pegasus \
    pegasus_media \
    favorites \
    logiqx \
    playtime \

win32: SUBDIRS += launchbox
