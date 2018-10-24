#! /bin/bash

set -o errexit
set -o nounset
set -o xtrace


export QT_QPA_PLATFORM=minimal

sudo add-apt-repository -y ppa:beineri/opt-qt58-trusty
sudo apt-get update
sudo apt-get install -y \
  lcov \
  qt58declarative \
  qt58graphicaleffects \
  qt58gamepad \
  qt58imageformats \
  qt58multimedia \
  qt58svg \
  qt58tools \
  libpulse-dev

gem install coveralls-lcov
