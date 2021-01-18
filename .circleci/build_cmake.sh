#! /bin/bash

set -o errexit
set -o nounset
set -o xtrace

QT_DIR=/opt/${QT_VERSION}_${QT_TARGET}_hosttools
if [[ ! -d "$QT_DIR" ]]; then
  QT_DIR=/opt/${QT_VERSION}_${QT_TARGET}
fi

SDL_DIR=""
if [[ -n "${SDL_VERSION:-}" ]]; then
  SDL_DIR="/opt/SDL2-${SDL_VERSION}"
fi

mkdir build && pushd build
eval /opt/cmake/bin/cmake .. \
  -DCMAKE_PREFIX_PATH="$QT_DIR\;$SDL_DIR" \
  -G Ninja \
  -DCMAKE_CXX_FLAGS=-fdiagnostics-color=always \
  ${BUILDOPTS:-} \
  -DPEGASUS_ENABLE_APNG=ON \
  -DPEGASUS_STATIC_CXX=ON \

ninja -j3  # The default executor on Circle CI has 2 cores and 4GB RAM
DESTDIR="${PWD}/../installdir" ninja install/strip
popd
