DEFINES *= WITH_SDL_GAMEPAD

isEmpty(SDL_LIBS):isEmpty(SDL_INCLUDES) {
    unix|win32-g++: {
        CONFIG += link_pkgconfig
        PKGCONFIG += sdl2
    }
    else: error("Please set SDL_LIBS and SDL_INCLUDES")
}
else {
    LIBS += $${SDL_LIBS}
    INCLUDEPATH += $${SDL_INCLUDES}
    DEPENDPATH += $${SDL_INCLUDES}
}
