#! /bin/bash

set -o errexit
set -o nounset

if [[ -z "$QT_VER" || -z "$TARGET" ]]; then
  echo "Please define QT_VER and TARGET first"
  exit 1
fi

set -o xtrace

# Build

mkdir build && cd build
/opt/${QT_VER}_${TARGET}_hosttools/bin/qmake .. \
  INSTALL_BINDIR=/usr/bin \
  INSTALL_ICONDIR=/usr/share/icons/hicolor/128x128/apps \
  INSTALL_DESKTOPDIR=/usr/share/applications

make
if [[ $TARGET == x11* ]]; then
  make check
fi

make install INSTALL_ROOT=$PWD/../installoc


cd ..
