#! /bin/bash

set -o errexit
set -o nounset

if [[ -z "$QT_VER" || -z "$TARGET" ]]; then
  echo "Please define QT_VER and TARGET first"
  exit 1
fi

set -o xtrace


# Lint

if [[ $TARGET == x11* ]]; then
  find -name *.qml -exec /opt/${QT_VER}_${TARGET}/bin/qmllint {} \;
fi

# Build

mkdir build && cd build
/opt/${QT_VER}_${TARGET}_hosttools/bin/qmake .. \
  INSTALL_BINDIR=/usr/bin \
  INSTALL_ICONDIR=/usr/share/icons/hicolor/128x128/apps \
  INSTALL_DESKTOPDIR=/usr/share/applications

make
if [[ $TARGET == x11* ]]; then
  export DISPLAY=:99.0
  sh -e /etc/init.d/xvfb start
  sleep 3
  make check
fi

make install INSTALL_ROOT=$PWD/../installoc


cd ..
