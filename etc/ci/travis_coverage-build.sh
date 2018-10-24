#! /bin/bash

echo "Calling '/opt/qt58/bin/qt58-env.sh'..."
source /opt/qt58/bin/qt58-env.sh

set -o errexit
set -o nounset
set -o xtrace

export DISPLAY=:99.0
sh -e /etc/init.d/xvfb start
sleep 3


qmake -v

mkdir build && cd build
qmake .. \
  QMAKE_CXXFLAGS="-g -O0 --coverage -fprofile-arcs -ftest-coverage" \
  QMAKE_LDFLAGS="-g -O0 --coverage -fprofile-arcs -ftest-coverage" \
  LIBS+="-lgcov" \
  CONFIG+=debug
make

lcov --compat-libtool -i -c -d . -o coverage.pre
make check
lcov --compat-libtool -c -d . -o coverage.post

set +o xtrace
lcov --compat-libtool -a coverage.pre -a coverage.post -o coverage.total
lcov --compat-libtool -r coverage.total \
  '/usr/*' \
  '/opt/*' \
  '*/moc_*' \
  '*/qrc_*' \
  '*/test_*' \
  '*/bench_*' \
  'src/app/*' \
  -o coverage.clean
sed -i 's|SF:/home/travis/build/mmatyas/pegasus-frontend/|SF:|g' coverage.clean


cd ..
coveralls-lcov build/coverage.clean
