#! /bin/bash

set -o errexit
set -o nounset
set -o xtrace

git submodule sync
git submodule update --init --recursive

mkdir dist


TOOLS_URL=https://github.com/mmatyas/pegasus-frontend/releases/download/alpha1
curl -L ${TOOLS_URL}/${QT_VERSION}_${QT_TARGET}.tar.xz | tar xJf - -C /opt/
