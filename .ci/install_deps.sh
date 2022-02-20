#! /bin/bash

set -o nounset
set -o errexit
set -o pipefail


if [[ -z "${QT_VERSION-}" || -z "${QT_TARGET-}" ]]; then
    echo "Please set the env vars QT_VERSION and QT_TARGET"
    exit 1
fi


ARG_BUILDSYS="qmake"
TOOLS_URL=https://github.com/mmatyas/pegasus-frontend/releases/download/alpha1


while [ $# -gt 0 ]; do
    option="$1"; shift
    case "$option" in
        --cmake)
            ARG_BUILDSYS="cmake"
        ;;
    esac
done


print_status() {
    echo -e "\e[94m${1}\e[0m"
}


get_tool() {
    local filename="$1"
    local outdir="$2"

    print_status "Downloading '${filename}' to '${outdir}'"
    curl -L "${TOOLS_URL}/${filename}.tar.xz" | tar xJf - -C "$outdir"
}


install_system_packages() {
    if [[ "$QT_TARGET" == macos* ]]; then
        return 0
    fi

    local packages="\
        build-essential \
        curl \
        g++ \
        git \
        pkg-config \
        unzip \
        zip \
    "
    [[ $ARG_BUILDSYS == cmake ]] && packages="$packages \
        ninja-build \
    "
    [[ "$QT_TARGET" == win32* ]] && packages="$packages \
        g++-mingw-w64-i686 \
    "
    [[ "$QT_TARGET" == rpi* ]] && packages="$packages \
        g++-arm-linux-gnueabihf \
    "
    [[ "$QT_TARGET" == odroid-* ]] && packages="$packages \
        g++-aarch64-linux-gnu \
        g++-arm-linux-gnueabihf \
        patchelf \
    "
    [[ "$QT_TARGET" == android* ]] && packages="$packages \
        openjdk-8-jdk \
        ssh-client \
    "
    [[ "$QT_TARGET" == x11* ]] && packages="$packages \
        libasound-dev \
        libgl1-mesa-dev \
        libgstreamer-plugins-base1.0-dev \
        libpulse-dev \
        libudev-dev \
        libxi-dev \
        libxkbcommon-dev \
        libxkbcommon-x11-dev \
        libxcb-icccm4-dev \
        libxcb-image0-dev \
        libxcb-keysyms1-dev \
        libxcb-render-util0-dev \
        libxcb-util-dev \
        libxcb-xinerama0-dev \
        libzstd-dev \
        ruby \
        xvfb \
    "

    print_status "Installing APT packages"
    apt-get update
    apt-get install -y $packages

    if [[ "$QT_TARGET" == win32* ]]; then
        echo 1 | update-alternatives --config i686-w64-mingw32-gcc
        echo 1 | update-alternatives --config i686-w64-mingw32-g++
    fi

    if [[ $ARG_BUILDSYS == cmake ]]; then
        get_tool cmake-linux-x64 /opt/
        /opt/cmake/bin/cmake --version
    fi
}


install_sysroot() {
    local tool=""
    [[ "$QT_TARGET" == rpi* ]] && tool=rpi-sysroot_buster_brcm
    [[ "$QT_TARGET" == rpi4* ]] && tool=rpi-sysroot_buster_mesa
    [[ "$QT_TARGET" == odroid-c1* ]] && tool=odroid-c1_bionic
    [[ "$QT_TARGET" == odroid-c2* ]] && tool=odroid-c2_bionic
    [[ "$QT_TARGET" == odroid-xu3* ]] && tool=odroid-5422s_bionic

    if [[ -n "$tool" ]]; then
        get_tool "$tool" /opt/
    fi
}


install_qt() {
    local installdir="/opt/qt${QT_VERSION//.}_${QT_TARGET}"
    [[ "$QT_TARGET" == macos* ]] && installdir="/usr/local/Qt-${QT_VERSION}"

    get_tool "qt${QT_VERSION//.}_${QT_TARGET}" "$(dirname "$installdir")"
    get_tool "apng_patched_headers" "${installdir}/include/"
}


install_sdl() {
    if [[ -z "${SDL_VERSION-}" || -z "${SDL_TARGET-}" ]]; then
      echo "SDL version or target not set, not installing SDL"
      return 0
    fi

    local install_root="/opt/"
    [[ "$SDL_TARGET" == macos* ]] && install_root="/usr/local/"

    get_tool "SDL2-${SDL_VERSION}_${SDL_TARGET}" "${install_root}"
}


install_ssl() {
    local tool=""
    [[ "$QT_TARGET" == win32* ]] && tool="openssl-111d_mingw"
    [[ "$QT_TARGET" == android* ]] && tool="openssl-111d_android"

    if [[ -n "$tool" ]]; then
        get_tool "$tool" /opt/
    fi
}


install_system_packages
install_sysroot
install_qt
install_sdl
install_ssl
