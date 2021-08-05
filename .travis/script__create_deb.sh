#! /bin/bash

set -o errexit
set -o nounset
set -o xtrace

SUFFIX=$(git describe --always)_${QT_TARGET}
DEB_REGEX_LATEST='^alpha([0-9]+)-([0-9]+)-g[0-9a-f]+_.*'
DEB_REGEX_STABLE='^alpha([0-9]+)_.*'

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
  -d 'libasound2' \
  -d 'libc6 >= 2.25' \
  -d 'libgl1' \
  -d 'libssl1.1' \
  -d 'libx11-6' \
  -d 'libx11-xcb1' \
  -d 'libxcb-glx0' \
  -d 'libxcb-icccm4' \
  -d 'libxcb-image0' \
  -d 'libxcb-keysyms1' \
  -d 'libxcb-randr0' \
  -d 'libxcb-render-util0' \
  -d 'libxcb-render0' \
  -d 'libxcb-shape0' \
  -d 'libxcb-shm0' \
  -d 'libxcb-sync1' \
  -d 'libxcb-xfixes0' \
  -d 'libxcb-xinerama0' \
  -d 'libxcb-xkb1' \
  -d 'libxcb1' \
  -d 'libxkbcommon-x11-0' \
  -d 'libxkbcommon0' \
  -d 'libzstd1' \
  -d 'policykit-1' \
  $PWD/../installdir/=/
