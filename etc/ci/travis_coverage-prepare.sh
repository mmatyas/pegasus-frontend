#! /bin/bash

set -o errexit
set -o nounset
set -o xtrace

DIR=$(dirname "$0")

source $DIR/travis_prepare.sh

sudo apt-get install -y lcov
gem install coveralls-lcov
