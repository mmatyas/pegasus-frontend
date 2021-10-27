#! /bin/bash

set -o errexit
set -o nounset
set -o xtrace


srcdir="$PWD/$1"


git_version=$(git describe --always)
deb_regex_latest='^alpha([0-9]+)-([0-9]+).+'
deb_regex_stable='^alpha([0-9]+).+'

deb_ver=''
if [[ "$git_version" =~ $deb_regex_latest ]]; then
  deb_ver=$(echo $git_version | sed -r "s/${deb_regex_latest}/0.\1.\2/")
elif [[ "$git_version" =~ $deb_regex_stable ]]; then
  deb_ver=$(echo $git_version | sed -r "s/${deb_regex_stable}/0.\1.0/")
else
  echo "Unrecognized version ${git_version}"
  exit 1
fi

mkdir -p deploy
cd deploy

PATH="$PATH:$HOME/.gem/ruby/2.5.0/bin/"
gem install --user-install -N fpm -v 1.10.2
fpm -s dir -t deb \
  --name 'pegasus-frontend' \
  --version ${deb_ver} \
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
  ${srcdir}/=/
