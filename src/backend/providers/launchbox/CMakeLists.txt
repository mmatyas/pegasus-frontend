pegasus_add_provider(
    NAME "LaunchBox"
    CXXID LAUNCHBOX
    SOURCES
        LaunchBoxAssets.cpp
        LaunchBoxAssets.h
        LaunchBoxEmulatorsXml.cpp
        LaunchBoxEmulatorsXml.h
        LaunchBoxGamelistXml.cpp
        LaunchBoxGamelistXml.h
        LaunchBoxPlatformsXml.cpp
        LaunchBoxPlatformsXml.h
        LaunchBoxProvider.cpp
        LaunchBoxProvider.h
        LaunchBoxXml.cpp
        LaunchBoxXml.h
    PLATFORMS
        WINDOWS
)
