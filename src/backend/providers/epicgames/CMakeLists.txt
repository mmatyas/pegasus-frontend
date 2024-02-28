pegasus_add_provider(
    NAME "Epic Games"
    CXXID EPICGAMES
    SOURCES
        EpicGamesGamelist.cpp
        EpicGamesGamelist.h
        EpicGamesProvider.cpp
        EpicGamesProvider.h
    USES_JSON_CACHE
    PLATFORMS
        WINDOWS
)
