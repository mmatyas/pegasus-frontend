#! /bin/bash

set -o nounset
set -o errexit
set -o pipefail


if [[ -z "${QT_VERSION-}" || -z "${QT_TARGET-}" ]]; then
    echo "Please set the env vars QT_VERSION and QT_TARGET"
    exit 1
fi


ARG_BUILDSYS="qmake"
ARG_RUNTESTS=0


while [ $# -gt 0 ]; do
    option="$1"; shift
    case "$option" in
        --cmake)
            ARG_BUILDSYS="cmake"
        ;;
        --with-tests)
            ARG_RUNTESTS=1
        ;;
        *)
            exit 1
        ;;
    esac
done


print_status() {
    echo -e "\e[94m${1}\e[0m"
}


find_qt() {
    QT_DIR=/opt/qt${QT_VERSION//.}_${QT_TARGET}
    [[ "$QT_TARGET" == macos* ]] && QT_DIR="/usr/local/Qt-${QT_VERSION}"

    QT_TOOLDIR="$QT_DIR"
    [[ -d "${QT_DIR}_hosttools" ]] && QT_TOOLDIR="${QT_DIR}_hosttools"

    return 0
}


before_build() {
    if [[ "$QT_TARGET" == rpi4* ]]; then
        print_status "Applying KMS launch fix"
        git apply ./etc/rpi4/kms_launch_fix.diff
        return 0
    fi
}


call_qmake() {
    local qmake_path="${QT_TOOLDIR}/bin/qmake"
    if [[ ! -f "$qmake_path" ]]; then
        echo "QMake not found at '$qmake_path'"
        exit 1
    fi

    "$qmake_path" --version

    local qmake_args="\
        QMAKE_CXXFLAGS+='-Wall -Wextra -pedantic' \
        ENABLE_APNG=1 \
    "

    if [[ -n "${SDL_VERSION-}" && -n "${SDL_TARGET-}" ]]; then
        qmake_args="${qmake_args} \
            USE_SDL_GAMEPAD=1 \
            USE_SDL_POWER=1 \
        "
        [[ "$QT_TARGET" == macos* ]] && qmake_args="${qmake_args} \
            SDL_LIBS='-L/usr/local/SDL2-${SDL_VERSION}/lib -lSDL2 -lm -liconv -Wl,-framework,ForceFeedback -lobjc -Wl,-framework,CoreVideo -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,IOKit -Wl,-framework,GameController' \
            SDL_INCLUDES=/usr/local/SDL2-${SDL_VERSION}/include/SDL2 \
            QMAKE_CFLAGS+=-D_THREAD_SAFE \
            QMAKE_CXXFLAGS+=-D_THREAD_SAFE \
        "
        [[ "$QT_TARGET" == win32* ]] && qmake_args="${qmake_args} \
            SDL_LIBS='-L/opt/SDL2-${SDL_VERSION}/lib/ -lSDL2 -Wl,--no-undefined -lm -ldinput8 -ldxguid -ldxerr8 -lsetupapi' \
            SDL_INCLUDES=/opt/SDL2-${SDL_VERSION}/include/SDL2/ \
        "
        [[ "$QT_TARGET" != macos* && "$QT_TARGET" != win32* ]] && qmake_args="${qmake_args} \
            SDL_LIBS='-L/opt/SDL2-${SDL_VERSION}/lib -lSDL2' \
            SDL_INCLUDES=/opt/SDL2-${SDL_VERSION}/include/SDL2 \
        "
    fi

    [[ "$QT_TARGET" == x11* ]] && qmake_args="${qmake_args} \
        INSTALL_BINDIR=/usr/bin \
        INSTALL_ICONDIR=/usr/share/icons/hicolor \
        INSTALL_DESKTOPDIR=/usr/share/applications \
        INSTALL_APPSTREAMDIR=/usr/share/metainfo \
        INSTALL_DOCDIR=/usr/share/doc/pegasus-frontend \
    "
    [[ "$QT_TARGET" == win32* ]] && qmake_args="${qmake_args} \
        INSTALLDIR=/pegasus-frontend \
    "
    [[ "$QT_TARGET" == android* ]] && qmake_args="${qmake_args} \
        ANDROID_ABIS=armeabi-v7a \
        FORCE_QT_PNG=1 \
    "
    [[ "$QT_TARGET" == rpi* ]] && qmake_args="${qmake_args} \
        QMAKE_LIBS_LIBDL=-ldl \
    "

    print_status "Calling QMake: '$qmake_path'"
    print_status "           in: '$PWD'"
    print_status "    with args: $qmake_args"

    eval "$qmake_path" .. $qmake_args
}


call_make() {
    print_status "Calling Make (build)"
    make

    print_status "Calling Make (install)"
    make install INSTALL_ROOT="${PWD}/../installdir"

    if [[ $ARG_RUNTESTS -gt 0 ]]; then
        print_status "Calling Make (test)"
        if [[ "$QT_TARGET" == macos* ]]; then
            make check
        else
            xvfb-run -a make check
        fi
    fi
}


call_cmake() {
    local cmake_path="/opt/cmake/bin/cmake"

    local cmake_args="\
        -G Ninja
        -DCMAKE_CXX_FLAGS=-fdiagnostics-color=always
        -DPEGASUS_ENABLE_APNG=ON
    "
    if [[ -n "${SDL_VERSION-}" && -n "${SDL_TARGET-}" ]]; then
        cmake_args="${cmake_args} \
            -DCMAKE_PREFIX_PATH='${QT_DIR};/opt/SDL2-${SDL_VERSION}'
        "
    else
        cmake_args="${cmake_args} \
            -DCMAKE_PREFIX_PATH='${QT_DIR}'
        "
    fi
    [[ "$QT_TARGET" == x11* ]] && cmake_args="${cmake_args} \
        -DCMAKE_INSTALL_PREFIX=/usr/
        -DPEGASUS_STATIC_CXX=ON
    "

    print_status "Calling CMake: '$cmake_path'"
    print_status "           in: '$PWD'"
    print_status "    with args: $cmake_args"

    eval "$cmake_path" .. $cmake_args
}


call_ninja() {
    print_status "Calling Ninja (build)"
    ninja

    print_status "Calling Ninja (install)"
    DESTDIR="${PWD}/../installdir" ninja install/strip

    if [[ $ARG_RUNTESTS -gt 0 ]]; then
        print_status "Calling Ninja (test)"

        if [[ "$QT_TARGET" == macos* ]]; then
            ctest --rerun-failed --output-on-failure
        else
            xvfb-run -a ctest --rerun-failed --output-on-failure
        fi
    fi
}


show_deps() {
    if [[ "$QT_TARGET" == android* || "$QT_TARGET" == macos* ]]; then
        return 0
    fi

    print_status "External dependencies:"

    local exepath="installdir/opt/pegasus-frontend/pegasus-fe"
    [[ "$QT_TARGET" == win32* ]] && exepath="installdir/pegasus-frontend/pegasus-fe.exe"
    [[ "$QT_TARGET" == x11* ]] && exepath="installdir/usr/bin/pegasus-fe"

    local triplet=""
    [[ "$QT_TARGET" == rpi* || "$QT_TARGET" == odroid* ]] && triplet="arm-linux-gnueabihf-"
    [[ "$QT_TARGET" == odroid-c2* ]] && triplet="aarch64-linux-gnu-"
    [[ "$QT_TARGET" == win32* ]] && triplet="i686-w64-mingw32-"

    local marker="NEEDED"
    [[ "$QT_TARGET" == win32* ]] && marker="DLL Name"

    ${triplet}objdump -p "$exepath" | grep "$marker" | sort
}


after_build() {
    if [[ "$QT_TARGET" == odroid* ]]; then
        print_status "Patching hardcoded libEGL"

        local triplet="arm-linux-gnueabihf-"
        [[ "$QT_TARGET" == odroid-c2* ]] && triplet="aarch64-linux-gnu-"

        local sysroot=""
        [[ "$QT_TARGET" == odroid-c1* ]] && sysroot="odroid-c1_bionic"
        [[ "$QT_TARGET" == odroid-c2* ]] && sysroot="odroid-c2_bionic"
        [[ "$QT_TARGET" == odroid-xu3* ]] && sysroot="odroid-5422s_bionic"

        patchelf --replace-needed \
            /opt/${sysroot}/usr/lib/${triplet}/libEGL.so libEGL.so \
            installdir/opt/pegasus-frontend/pegasus-fe

        return 0
    fi
}


find_qt
before_build

mkdir build
pushd build

if [[ $ARG_BUILDSYS == qmake ]]; then
    call_qmake
    call_make
fi
if [[ $ARG_BUILDSYS == cmake ]]; then
    call_cmake
    call_ninja
fi

popd

show_deps
after_build
