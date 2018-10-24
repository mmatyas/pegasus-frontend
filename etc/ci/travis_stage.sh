#! /bin/bash

set -o errexit
set -o nounset

if [[ -z "$QT_VER" || -z "$TARGET" ]]; then
  echo "Please define QT_VER and TARGET first"
  exit 1
fi

set -o xtrace

SUFFIX=$(git describe --always)_${TARGET}
DEB_REGEX_LATEST='^alpha([0-9]+)-([0-9]+)-g[0-9a-f]+_'
DEB_REGEX_STABLE='^alpha([0-9]+)_'


# Regular release

mkdir dist && cd dist
zip -j pegasus-fe_${SUFFIX}.zip \
  ../installoc/usr/bin/pegasus-fe \
  ../README.md \
  ../LICENSE.md


# DEB package

if [[ $TARGET = x11* ]]; then
  if [[ "$SUFFIX" =~ $DEB_REGEX_LATEST ]]; then
    DEB_VER=$(echo $SUFFIX | sed -r "s/${DEB_REGEX_LATEST}/0.\1.\2/")
  elif [[ "$SUFFIX" =~ $DEB_REGEX_STABLE ]]; then
    DEB_VER=$(echo $SUFFIX | sed -r "s/${DEB_REGEX_STABLE}/0.\1.0/")
  else
    echo "Unrecognized suffix ${SUFFIX}"
    exit 1
  fi

  fpm -s dir -t deb \
    --name 'pegasus-frontend' \
    --version ${DEB_VER} \
    --license 'GPLv3+, CC-BY-NC-SA' \
    --maintainer 'Mátyás Mustoha' \
    --vendor 'pegasus-frontend.org' \
    --url 'http://pegasus-frontend.org' \
    --category games \
    --deb-compression xz \
    --description "$(printf 'Cross platform graphical frontend for launching emulators\nPegasus is an experimental emulator frontend, focusing on customizability,\ncross platform support (including embedded) and high performance.')" \
    -d 'fontconfig' \
    -d 'gstreamer1.0-plugins-good' \
    -d 'policykit-1' \
    -d 'libc6 >= 2.17' \
    -d 'libstdc++6 >= 4.8' \
    $PWD/../installoc/=/
fi


cd ..
