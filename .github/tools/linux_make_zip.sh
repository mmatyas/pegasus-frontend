#! /bin/bash

set -o errexit
set -o nounset
set -o xtrace


zipsuffix="$1"
exefile="$2"

mkdir -p deploy
zip -j deploy/pegasus-fe_$(git describe --always)_${zipsuffix}.zip \
    "${exefile}" \
    README.md \
    LICENSE.md
