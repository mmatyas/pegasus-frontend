#! /bin/bash

set -o nounset
set -o errexit
set -o pipefail


if [[ -z "${QT_VERSION-}" || -z "${QT_TARGET-}" ]]; then
    echo "Please set the env vars QT_VERSION and QT_TARGET"
    exit 1
fi


EXEPATH="installdir/opt/pegasus-frontend/pegasus-fe"
[[ "$QT_TARGET" == win32* ]] && EXEPATH="installdir/pegasus-frontend/pegasus-fe.exe"
[[ "$QT_TARGET" == x11* ]] && EXEPATH="installdir/usr/bin/pegasus-fe"
[[ "$QT_TARGET" == macos* ]] && EXEPATH="installdir/usr/local/pegasus-frontend/Pegasus.app"

RELEASE_NAME="pegasus-fe_$(git describe --always)"
ZIP_NAME="${RELEASE_NAME}_${QT_TARGET}.zip"
echo "Creating release '$PWD/deploy/${ZIP_NAME}'"

mkdir staging deploy
mv README.md LICENSE.md "$EXEPATH" staging/
pushd staging
zip -r "../deploy/${ZIP_NAME}" ./*
popd
