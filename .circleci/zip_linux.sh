#! /bin/bash

set -o errexit
set -o nounset
set -o xtrace

zip -j dist/pegasus-fe_$(git describe --always)_${QT_TARGET}.zip \
    installdir/opt/pegasus-frontend/pegasus-fe \
    README.md \
    LICENSE.md
