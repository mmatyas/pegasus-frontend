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
    ruby2.4
  gem install fpm -v 1.10.2
fi

if [[ -n ${RUN_COV-} ]]; then
  sudo apt-get install -y lcov
  gem install coveralls-lcov
fi


# Install the toolchain

TOOLS_URL=https://github.com/mmatyas/pegasus-frontend/releases/download/alpha1

pushd /tmp
  wget ${TOOLS_URL}/${QT_VER}_${TARGET}.txz
  if [[ $TARGET = rpi* ]]; then
    if [[ $TARGET = rpi1* ]];
      then wget ${TOOLS_URL}/rpi-toolchain-official.txz
      else wget ${TOOLS_URL}/rpi-toolchain-linaro.txz
    fi
    wget ${TOOLS_URL}/rpi-sysroot_brcm493fix.txz
  fi

  for f in *.txz; do sudo tar xJf ${f} -C /opt/; done

  if [[ $TARGET = x11* ]]; then sudo ln -s ${QT_VER}_${TARGET} /opt/${QT_VER}_${TARGET}_hosttools; fi
popd
