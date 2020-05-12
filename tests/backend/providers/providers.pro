TEMPLATE = subdirs

SUBDIRS += \
    pegasus \
    favorites \
    playtime \

win32: SUBDIRS += launchbox
