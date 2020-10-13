pegasus_add_provider(
    NAME "EmulationStation"
    CXXID ES2
    SOURCES
        Es2Games.cpp
        Es2Games.h
        Es2Metadata.cpp
        Es2Metadata.h
        Es2Provider.cpp
        Es2Provider.h
        Es2Systems.cpp
        Es2Systems.h
    PLATFORMS
        WINDOWS
        MACOS
        X11
        EGLFS
)
