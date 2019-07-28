#! /bin/bash

set -o errexit
set -o nounset

if [[ -z "${QT_HOSTDIR-}" ]]; then
  echo "Please define QT_HOSTDIR first"
  exit 1
fi


${QT_HOSTDIR}/bin/qmake .. \
  QMAKE_CXXFLAGS="-g -O0 --coverage -fprofile-arcs -ftest-coverage" \
  QMAKE_LDFLAGS="-g -O0 --coverage -fprofile-arcs -ftest-coverage" \
  LIBS+="-lgcov" \
  CONFIG+=debug
make


lcov --compat-libtool -i -c -d . -o coverage.pre
make check
lcov --compat-libtool -c -d . -o coverage.post


lcov --compat-libtool -a coverage.pre -a coverage.post -o coverage.total
PARENTDIR="$(dirname "$(pwd)")"
sed -i "s|SF:$PARENTDIR/|SF:|g" coverage.total

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
