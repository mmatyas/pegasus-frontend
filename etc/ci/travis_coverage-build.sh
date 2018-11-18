#! /bin/bash

set -o errexit
set -o nounset

if [[ -z "$QT_VER" || -z "$TARGET" ]]; then
  echo "Please define QT_VER and TARGET first"
  exit 1
fi
if [[ $TARGET != x11* ]]; then
  echo "This script support X11 only at the moment"
  exit 1
fi


set -o xtrace

export DISPLAY=:99.0
sh -e /etc/init.d/xvfb start
sleep 3


# Lint

find -name *.qml -exec /opt/${QT_VER}_${TARGET}/bin/qmllint {} \;


# Build

/opt/${QT_VER}_${TARGET}_hosttools/bin/qmake -v

mkdir build && cd build
/opt/${QT_VER}_${TARGET}_hosttools/bin/qmake .. \
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
