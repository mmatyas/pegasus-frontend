#! /bin/bash

set -o errexit
set -o nounset
set -o xtrace

${1}objdump -p installdir/opt/pegasus-frontend/pegasus-fe | grep 'NEEDED' | sort
