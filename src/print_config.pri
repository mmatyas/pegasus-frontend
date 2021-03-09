# Print deployment information

defineTest(printKeyVal) {
    message("  - $$1: $$2")
    return(true)
}

defineTest(printOptVal) {
    name = $$1
    value = "unset, will not install"
    !isEmpty($$2): value = `$$eval($$2)`
    printKeyVal($${name}, $${value})
}


message("Deployment (`make install`) paths:")

printOptVal("Binaries", INSTALL_BINDIR)
printOptVal("License and Readme", INSTALL_DOCDIR)
unix:!macx {
    printOptVal("X11: Icon files", INSTALL_ICONDIR)
    printOptVal("X11: Desktop file", INSTALL_DESKTOPDIR)
    printOptVal("X11: AppStream file", INSTALL_APPSTREAMDIR)
}


message("Additional components")

# Gamepad backend
!isEmpty(USE_SDL_GAMEPAD): printKeyVal("Gamepad backend", "SDL2")
else: printKeyVal("Gamepad backend", "Qt")

# Battery info backend
!isEmpty(USE_SDL_POWER): printKeyVal("Battery info backend", "SDL2")
else:android: printKeyVal("Battery info backend", "Android")
else: printKeyVal("Battery info backend", "DISABLED")

# APNG support
!isEmpty(ENABLE_APNG): printKeyVal("APNG support", "Enabled")
else: printKeyVal("APNG support", "DISABLED")


# SDL2
!isEmpty(USE_SDL_GAMEPAD)|!isEmpty(USE_SDL_POWER): {
    message("Linking to SDL2")
    isEmpty(SDL_LIBS):isEmpty(SDL_INCLUDES) {
        unix|win32-g++: message("  - using pkg-config to find it")
        else: error("Please set SDL_LIBS and SDL_INCLUDES")
    }
    else {
        message("  - Libraries:")
        for(part, SDL_LIBS): message("    - $${part}")
        message("  - Include paths:")
        for(path, SDL_INCLUDES): message("    - $${path}")
    }
}


# Print Git revision
message("Git revision: '$${GIT_REVISION}'")
