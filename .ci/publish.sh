#! /bin/bash

set -o errexit
set -o nounset
set -o pipefail
set +x


STAGING_REPO=https://github.com/mmatyas/pegasus-deploy-staging.git
GIT_REV=$(git describe --always)
TARGETS=" \
    x11-static \
    rpi1-static \
    rpi2-static \
    rpi3-static \
    rpi4-static \
    macos-static \
    android \
    win32-mingw-static \
    odroid-c1-static \
    odroid-c2-static \
    odroid-xu3-static \
"
EXPECTED_FILE_CNT=$(echo $TARGETS | wc -w)  # regular releases
((EXPECTED_FILE_CNT++))  # + 1 deb


# Collect

FILE_CNT=0
NEXT_SLEEP_SEC=0

while [[ $FILE_CNT -ne $EXPECTED_FILE_CNT ]]; do
    if [[ $NEXT_SLEEP_SEC -gt 0 ]]; then
        echo "Retrying in $NEXT_SLEEP_SEC seconds..."
        sleep $NEXT_SLEEP_SEC
    fi

    for target in $TARGETS; do
        rm -rf "dist-${target}"
        git clone ${STAGING_REPO} -b "continuous-${target}" --depth=1 "dist-${target}"
        target_files=$(find "dist-${target}" -maxdepth 2 -name "*${GIT_REV}*.zip" -or -name "*.deb" -or -name "*${GIT_REV}*.apk")
        if [[ -n "$target_files" ]]; then
            echo "Found files: $target_files"
            echo ""
        else
            echo "no files found"
            echo ""
            break
        fi
    done

    FILES=$(find ./ -maxdepth 2 -name "*${GIT_REV}*.zip" -or -name "*.deb" -or -name "*.apk");
    FILE_CNT=$(echo $FILES | wc -w)
    echo "Available files:"
    echo ${FILES}
    echo "(${FILE_CNT} out of ${EXPECTED_FILE_CNT})";

    NEXT_SLEEP_SEC=20
done


# Deploy

mkdir deploy
mv $FILES deploy/
curl -L https://github.com/mmatyas/pegasus-frontend/releases/download/alpha1/ghr_v0.14.0_linux_amd64.tar.gz | tar xzf -
ghr_v0.14.0_linux_amd64/ghr -replace -prerelease continuous deploy
