pegasus_add_provider(
    NAME "Steam"
    CXXID STEAM
    SOURCES
        SteamGamelist.cpp
        SteamGamelist.h
        SteamMetadata.cpp
        SteamMetadata.h
        SteamProvider.cpp
        SteamProvider.h
    USES_JSON_CACHE
    PLATFORMS
        WINDOWS
        MACOS
        X11
)
