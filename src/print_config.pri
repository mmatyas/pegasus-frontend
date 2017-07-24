# Print deployment information
message("Deployment (`make install`) paths:")
message("  - Default (`INSTALLDIR`): `$${INSTALLDIR}`")
message("  - Binaries (`INSTALL_BINDIR`): `$${INSTALL_BINDIR}`")
message("  - Data files (`INSTALL_DATADIR`): `$${INSTALL_DATADIR}`")
unix:!macx {
    message("  - X11: Icon files (`INSTALL_ICONDIR`): `$${INSTALL_ICONDIR}`")
    message("  - X11: Desktop file (`INSTALL_DESKTOPDIR`): `$${INSTALL_DESKTOPDIR}`")
}


# Print Git revision
message("Git revision: '$${GIT_REVISION}'")
