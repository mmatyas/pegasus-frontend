#! /bin/bash

set -o errexit
set -o nounset

if [[ "$CIRCLE_BRANCH" != "master" ]]; then
  echo "Skipping deployment for pull requests"
  exit 0
fi


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
  android64 \
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
  if [[ $NEXT_SLEEP_SEC -ne 0 ]]; then echo Retrying in $NEXT_SLEEP_SEC seconds...; fi
  sleep $NEXT_SLEEP_SEC
  NEXT_SLEEP_SEC=20

  for target in $TARGETS; do
    rm -rf dist-${target}
    git clone ${STAGING_REPO} -b continuous-${target} --depth=1 dist-${target}
    FILES=$(find "dist-${target}" -maxdepth 2 -name "*${GIT_REV}*.zip" -or -name "*.deb" -or -name "*${GIT_REV}*.apk")
    if [[ ! -z "$FILES" ]]; then
      echo "Found files: $FILES"
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
done

curl -LO https://github.com/mmatyas/uploadtool/raw/circleci/upload.sh
bash ./upload.sh $FILES
