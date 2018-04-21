# Print deployment information
message("Deployment (`make install`) paths:")
message("  - Default (`INSTALLDIR`): `$${INSTALLDIR}`")
message("  - Binaries (`INSTALL_BINDIR`): `$${INSTALL_BINDIR}`")
message("  - Data files (`INSTALL_DATADIR`): `$${INSTALL_DATADIR}`")

unix:!macx {
    isEmpty(INSTALL_ICONDIR) {
        message("  - X11: Icon file (`INSTALL_ICONDIR`): not set, skipped")
    } else {
        message("  - X11: Icon file (`INSTALL_ICONDIR`): `$${INSTALL_ICONDIR}`")
    }

    isEmpty(INSTALL_DESKTOPDIR) {
        message("  - X11: Desktop file (`INSTALL_DESKTOPDIR`): not set, skipped")
    } else {
        message("  - X11: Desktop file (`INSTALL_DESKTOPDIR`): `$${INSTALL_DESKTOPDIR}`")
    }
}


# Print Git revision
message("Git revision: '$${GIT_REVISION}'")
