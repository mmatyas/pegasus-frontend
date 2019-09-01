#! /bin/bash

set -o errexit
set -o nounset
set -o xtrace

QT_DIR=/opt/${QT_VERSION}_${QT_TARGET}_hosttools
if [[ ! -d "$QT_DIR" ]]; then
  QT_DIR=/opt/${QT_VERSION}_${QT_TARGET}
fi

BUILDOPTS=""
if [ -n "${SDL_ROOT:-}" ]; then
  BUILDOPTS+=" USE_SDL_GAMEPAD=1"
  BUILDOPTS+=" SDL_LIBS='-L${SDL_ROOT}/lib/ ${SDL_LIBS:-}'"
  BUILDOPTS+=" SDL_INCLUDES='${SDL_ROOT}/include/SDL2/'"
fi

${QT_DIR}/bin/qmake --version
eval ${QT_DIR}/bin/qmake . ${BUILDOPTS:-}
make
make install INSTALL_ROOT=${PWD}/installdir
