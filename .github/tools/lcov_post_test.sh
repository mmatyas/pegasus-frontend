#! /bin/bash

set -o errexit
set -o nounset
set -o xtrace

builddir="$1"
cd "${builddir}"

lcov --compat-libtool --capture --directory . -o coverage.post
lcov --compat-libtool -a coverage.pre -a coverage.post -o coverage.total

parentdir="$(dirname "$(pwd)")"
sed -i "s|SF:$parentdir/|SF:|g" coverage.total

lcov --compat-libtool -r coverage.total \
    '/usr/*' \
    '/opt/*' \
    '*/moc_*' \
    '*/qrc_*' \
    '*/test_*' \
    '*/bench_*' \
    'src/app/*' \
    'tests/*' \
    'thirdparty/*' \
    '*/pegasus-fe_plugin_import.cpp' \
    '*/pegasus-fe_qml_plugin_import.cpp' \
    '*/qmlcache_loader.cpp' \
    -o coverage.clean
