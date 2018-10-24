#! /bin/bash

set -o errexit
set -o nounset

if [[ -z "$QT_VER" || -z "$TARGET" ]]; then
  echo "Please define QT_VER and TARGET first"
  exit 1
fi

set -o xtrace

TOOLS_URL=https://github.com/mmatyas/pegasus-frontend/releases/download/alpha1


# Install the toolchain

pushd /tmp
  wget ${TOOLS_URL}/${QT_VER}_${TARGET}.txz
  if [[ $TARGET = rpi* ]]; then wget ${TOOLS_URL}/rpi-toolchain_483.txz; fi
  if [[ $TARGET = rpi* ]]; then wget ${TOOLS_URL}/rpi-sysroot_jessie_2017-07-05.txz; fi

  for f in *.txz; do sudo tar xJf ${f} -C /opt/; done

  if [[ $TARGET = x11* ]]; then sudo ln -s ${QT_VER}_${TARGET} /opt/${QT_VER}_${TARGET}_hosttools; fi
popd


# X11 native dependencies

if [[ $TARGET = x11* ]]; then
  sudo apt-add-repository -y ppa:brightbox/ruby-ng
  sudo apt-get -qq update
  sudo apt-get install -y \
    libasound-dev \
    libgstreamer-plugins-base1.0-dev \
    libpulse-dev \
    libudev-dev \
    libxi-dev \
    ruby2.4
  gem install fpm
fi
