# Print deployment information

defineTest(printSimpleOpt) {
    name = $$1
    value = "unset, will not install"
    !isEmpty($$2): value = `$$eval($$2)`

    message("  - $${name}: $${value}")
    return(true)
}

message("Deployment (`make install`) paths:")
printSimpleOpt("Binaries", INSTALL_BINDIR)
printSimpleOpt("License and Readme", INSTALL_DOCDIR)


unix:!macx {
    printSimpleOpt("X11: Icon files", INSTALL_ICONDIR)
    printSimpleOpt("X11: Desktop file", INSTALL_DESKTOPDIR)
    printSimpleOpt("X11: AppStream file", INSTALL_APPSTREAMDIR)
}


# Gamepad backend
!isEmpty(USE_SDL_GAMEPAD) {
    message("Using SDL2 gamepad backend")
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
else: message("Using Qt gamepad backend")


# APNG support
!isEmpty(ENABLE_APNG): message("APNG support enabled")
else: message("APNG support disabled")


# Print Git revision
message("Git revision: '$${GIT_REVISION}'")
