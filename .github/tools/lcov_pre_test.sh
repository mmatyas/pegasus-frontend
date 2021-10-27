#! /bin/bash

set -o errexit
set -o nounset
set -o xtrace

builddir="$1"
cd "${builddir}"

lcov --compat-libtool --initial --capture --directory . -o coverage.pre
