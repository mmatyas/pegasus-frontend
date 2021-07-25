ENABLED_COMPATS += Playnite

DEFINES *= WITH_COMPAT_PLAYNITE

HEADERS += \
    $$PWD/PlayniteProvider.h \
    $$PWD/PlayniteJsonHelper.h \
    $$PWD/PlayniteLaunchHelper.h \
    $$PWD/PlayniteMetadataParser.h \
    $$PWD/PlayniteEmulator.h \
    $$PWD/PlayniteGame.h \
    $$PWD/PlayniteComponents.h \
    $$PWD/PlayniteMetadataParser.h \


SOURCES += \
    $$PWD/PlayniteProvider.cpp \
    $$PWD/PlayniteJsonHelper.cpp \
    $$PWD/PlayniteLaunchHelper.cpp \
    $$PWD/PlayniteMetadataParser.cpp \
