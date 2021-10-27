#! /bin/bash

set -o errexit
set -o nounset
set -o xtrace

tools_url="https://github.com/mmatyas/pegasus-frontend/releases/download/alpha1"
tool="$1"
targetdir="$2"

curl -L "${tools_url}/${tool}.tar.xz" | sudo tar xJf - -C "${targetdir}"
