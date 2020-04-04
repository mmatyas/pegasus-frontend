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
# Platform settings - install path
if [[ $TARGET == macos* ]]; then
  INSTALLED_RUNNABLE=usr/local/pegasus-frontend/Pegasus.app
  INSTALLED_BINARY=${INSTALLED_RUNNABLE}/Contents/MacOS/pegasus-fe
else
  INSTALLED_RUNNABLE=opt/pegasus-frontend/pegasus-fe
  INSTALLED_BINARY=${INSTALLED_RUNNABLE}
fi


# Build

mkdir build && pushd build

${QT_HOSTDIR}/bin/qmake .. \
  INSTALL_ICONDIR=/usr/share/icons/hicolor/128x128/apps \
  INSTALL_DESKTOPDIR=/usr/share/applications \
  INSTALL_APPSTREAMDIR=/usr/share/metainfo \
  INSTALL_DOCDIR=/usr/share/doc/pegasus-frontend \
  QMAKE_CXXFLAGS+=-fno-rtti
make
make install INSTALL_ROOT=$PWD/../installoc

popd


# Check deps
BINHEAD=$(objdump -p "installoc/${INSTALLED_BINARY}")
echo "${BINHEAD}" | grep NEEDED | sort


# Create artifacts
mkdir dist && pushd dist
  zip -j pegasus-fe_$(git describe --always)_${TARGET}.zip \
    "../installoc/${INSTALLED_RUNNABLE}" \
    ../README.md \
    ../LICENSE.md
popd

for FILE in dist/*; do
    timeout 5m curl --upload-file $FILE https://transfer.sh/$(basename $FILE) || true
    echo ""
done
