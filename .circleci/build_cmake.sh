#! /bin/bash

set -o errexit
set -o nounset
set -o xtrace

QT_DIR=/opt/${QT_VERSION}_${QT_TARGET}_hosttools
if [[ ! -d "$QT_DIR" ]]; then
  QT_DIR=/opt/${QT_VERSION}_${QT_TARGET}
fi

eval cmake \
  -DCMAKE_PREFIX_PATH="$QT_DIR" \
  ${BUILDOPTS:-} \
  -G Ninja \
  -DCMAKE_CXX_FLAGS=-fdiagnostics-color=always
  -DPEGASUS_ENABLE_APNG=1
  -DPEGASUS_STATIC=1
ninja
DESTDIR="${PWD}/installdir" ninja install/strip
