pegasus_add_provider(
    NAME "GOG"
    CXXID GOG
    SOURCES
        GogGamelist.cpp
        GogGamelist.h
        GogMetadata.cpp
        GogMetadata.h
        GogProvider.cpp
        GogProvider.h
    USES_JSON_CACHE
    PLATFORMS
        WINDOWS
        X11
)
