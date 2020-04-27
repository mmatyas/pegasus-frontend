#! /bin/bash

set -o errexit
set -o nounset

if [[ -z ${QT_VER-} || -z ${TARGET-} ]]; then
  echo "Please define QT_VER and TARGET first"
  exit 1
fi

set -o xtrace


# Native dependencies

if [[ $TARGET = x11* ]]; then
  sudo apt-add-repository -y ppa:brightbox/ruby-ng
  sudo apt-get -qq update
  sudo apt-get install -y \
    libasound-dev \
    libgl1-mesa-dev \
    libgstreamer-plugins-base1.0-dev \
    libpulse-dev \
    libudev-dev \
    libxi-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    libsdl2-dev \
    ruby
  gem install fpm -v 1.10.2
fi

if [[ $TARGET = rpi* ]]; then
  sudo apt-get install -y g++-arm-linux-gnueabihf
fi

if [[ -n ${RUN_COV-} ]]; then
  sudo apt-get install -y lcov
  gem install coveralls-lcov
fi


# Install the toolchain

TOOLS_URL=https://github.com/mmatyas/pegasus-frontend/releases/download/alpha1

pushd /tmp
  wget ${TOOLS_URL}/qt${QT_VER//./}_${TARGET}.tar.xz

  if [[ $TARGET = rpi* ]]; then
    if [[ $TARGET = rpi4* ]];
    then wget ${TOOLS_URL}/rpi-sysroot_buster_mesa.tar.xz
    else wget ${TOOLS_URL}/rpi-sysroot_buster_brcm.tar.xz
    fi
  fi

  if [[ $TARGET == macos* ]]; then OUTDIR=/usr/local; else OUTDIR=/opt; fi
  for f in *.tar.xz; do sudo tar xJf ${f} -C ${OUTDIR}/; done
popd
