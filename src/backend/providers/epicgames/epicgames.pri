ENABLED_COMPATS += EpicGames
USES_JSON_CACHE = yes

DEFINES *= WITH_COMPAT_EPICGAMES

HEADERS += \
    $$PWD/EpicGamesGamelist.h \
    $$PWD/EpicGamesProvider.h \

SOURCES += \
    $$PWD/EpicGamesGamelist.cpp \
    $$PWD/EpicGamesProvider.cpp \
