pegasus_add_provider(
    NAME "Android Apps"
    CXXID ANDROIDAPPS
    SOURCES
        AndroidAppsMetadata.cpp
        AndroidAppsMetadata.h
        AndroidAppsProvider.cpp
        AndroidAppsProvider.h
    USES_JSON_CACHE
    PLATFORMS
        ANDROID
)
