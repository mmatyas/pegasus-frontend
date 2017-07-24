# This file sets the default values of the deployment variables;
# these control where will your files end up when you call
# `make install` (or similar).
#
# You can change the values by appending KEY=VALUE pairs to the
# `qmake` call, or adding them as `Additional arguments` in
# Qt Creator.


# The base/fallback installation directory
unix:!macx:isEmpty(INSTALLDIR): INSTALLDIR = /opt/pegasus-frontend
win32:isEmpty(INSTALLDIR): INSTALLDIR = C:/pegasus-frontend

# Linux: installation is portable by default
# Windows: installation should always be portable (one-dir)
isEmpty(INSTALL_BINDIR): INSTALL_BINDIR = $${INSTALLDIR}
isEmpty(INSTALL_DATADIR): INSTALL_DATADIR = $${INSTALLDIR}

# X11 desktop and icon file location
unix:!macx {
    isEmpty(INSTALL_ICONDIR): INSTALL_ICONDIR = $${INSTALLDIR}
    isEmpty(INSTALL_DESKTOPDIR): INSTALL_DESKTOPDIR = $${INSTALLDIR}
}
