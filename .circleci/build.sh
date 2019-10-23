#! /bin/bash

set -o errexit
set -o nounset
set -o xtrace

QT_DIR=/opt/${QT_VERSION}_${QT_TARGET}_hosttools
if [[ ! -d "$QT_DIR" ]]; then
  QT_DIR=/opt/${QT_VERSION}_${QT_TARGET}
fi

${QT_DIR}/bin/qmake --version
eval ${QT_DIR}/bin/qmake . ${BUILDOPTS:-} QMAKE_CXXFLAGS+=-fno-rtti
make
make install INSTALL_ROOT=${PWD}/installdir
