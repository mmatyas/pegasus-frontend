#! /bin/bash

set -o errexit
set -o nounset

toolchain="$1"
exefile="$2"
marker="${3:-NEEDED}"

${toolchain}objdump -p "${exefile}" | grep "${marker}" | sort
