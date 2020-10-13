pegasus_add_provider(
    NAME "Playnite"
    CXXID PLAYNITE
    SOURCES
        PlayniteComponents.h
        PlayniteEmulator.h
        PlayniteGame.h
        PlayniteJsonHelper.cpp
        PlayniteJsonHelper.h
        PlayniteLaunchHelper.cpp
        PlayniteLaunchHelper.h
        PlayniteMetadataParser.cpp
        PlayniteMetadataParser.h
        PlayniteMetadataParser.h
        PlayniteProvider.cpp
        PlayniteProvider.h
    PLATFORMS
        WINDOWS
)
