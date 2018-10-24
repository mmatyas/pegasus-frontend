#! /bin/bash

set -o errexit
set -o nounset

if [[ $TRAVIS_EVENT_TYPE = pull_request ]]; then
  echo "Skipping deployment for pull requests"
  exit 0
fi


STAGING_REPO=https://github.com/mmatyas/pegasus-deploy-staging.git
GIT_REV=$(git describe --always)
TARGETS='x11-static rpi1-static rpi2-static macos-static'
EXPECTED_FILE_CNT=5  # 4 regular + 1 deb


# Collect

FILE_CNT=0
NEXT_SLEEP_SEC=0

while [[ $FILE_CNT -ne $EXPECTED_FILE_CNT ]]; do
  if [[ $NEXT_SLEEP_SEC -ne 0 ]]; then echo Retrying in $NEXT_SLEEP_SEC seconds...; fi
  sleep $NEXT_SLEEP_SEC
  NEXT_SLEEP_SEC=20

  for target in $TARGETS; do
    rm -rf dist-${target};
    git clone ${STAGING_REPO} -b continuous-${target} dist-${target};
  done;

  FILES=$(find ./ -maxdepth 2 -name "*${GIT_REV}*.zip" -or -name "*.deb");
  FILE_CNT=$(echo $FILES | wc -w);
  echo "Available files:"
  echo ${FILES}
  echo "(${FILE_CNT} out of ${EXPECTED_FILE_CNT})";
done


# Deploy

wget https://github.com/mmatyas/uploadtool/raw/master/upload.sh;
bash ./upload.sh $FILES
