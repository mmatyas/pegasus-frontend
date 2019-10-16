#! /bin/bash

set -o errexit
set -o nounset

if [[ -z ${QT_VER-} || -z ${TARGET-} ]]; then
  echo "Please define QT_VER and TARGET first"
  exit 1
fi
set -o xtrace


# Platform settings - Qt dir
if [[ $TARGET == macos* ]]; then
  QT_HOSTDIR=/usr/local/Qt-${QT_VER}
elif [[ $TARGET == x11* ]]; then
  QT_HOSTDIR=/opt/qt${QT_VER//./}_${TARGET}
else
  QT_HOSTDIR=/opt/qt${QT_VER//./}_${TARGET}_hosttools
fi
# Platform settings - Cross prefix
if [[ $TARGET == rpi1* ]]; then
  CROSS=/opt/rpi-toolchain/arm-bcm2708/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
elif [[ $TARGET == rpi* ]]; then
  CROSS=/opt/linaro/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
fi
# Platform settings - install path
if [[ $TARGET == macos* ]]; then
  INSTALLED_FILE=usr/local/pegasus-frontend/Pegasus.app
  INSTALLED_BINARY=${INSTALLED_FILE}/Contents/MacOS/pegasus-fe
else
  INSTALLED_FILE=usr/bin/pegasus-fe
  INSTALLED_BINARY=${INSTALLED_FILE}
fi


# Lint
find -name *.qml -exec ${QT_HOSTDIR}/bin/qmllint {} \;


# Build

mkdir build && pushd build

if [[ -n ${RUN_COV-} ]]; then
  source ../.travis/script__build_with_lcov.sh
  (cd .. && coveralls-lcov build/coverage.clean)
  popd
  exit 0
fi

${QT_HOSTDIR}/bin/qmake .. \
  USE_SDL_GAMEPAD=1 \
  INSTALL_BINDIR=/usr/bin \
  INSTALL_ICONDIR=/usr/share/icons/hicolor/128x128/apps \
  INSTALL_DESKTOPDIR=/usr/share/applications \
  INSTALL_APPSTREAMDIR=/usr/share/metainfo
make
make install INSTALL_ROOT=$PWD/../installoc

popd


# Check deps
BINHEAD=$(${CROSS-}objdump -p "installoc/${INSTALLED_BINARY}")
echo "${BINHEAD}" | grep NEEDED | sort


# Create artifacts
mkdir dist && pushd dist
  zip -j pegasus-fe_$(git describe --always)_${TARGET}.zip \
    "../installoc/${INSTALLED_FILE}" \
    ../README.md \
    ../LICENSE.md

  if [[ $TARGET = x11* ]]; then
    source ../.travis/script__create_deb.sh
  fi
popd

for FILE in dist/*; do
    timeout 5m curl --upload-file $FILE https://transfer.sh/$(basename $FILE) || true
    echo ""
done
