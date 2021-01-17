#! /bin/bash

set -o errexit
set -o nounset
set +o xtrace # !

shopt -s nullglob

if [[ "$CIRCLE_BRANCH" != "master" ]]; then
  echo "Skipping deployment for pull requests"
  exit 0
fi


GIT_TAG=$(git describe --always --abbrev=0)


for file in dist-x11-static/*.deb; do
    echo "Uploading '${file}' to Bintray"

    filename=$(basename "$file")
    curl \
        -T "${file}" \
        -H "X-Bintray-Package: pegasus-frontend" \
        -H "X-Bintray-Version: ${GIT_TAG}" \
        -H "X-Bintray-Publish: 1" \
        -H "X-Bintray-Debian-Distribution: testing" \
        -H "X-Bintray-Debian-Component: main" \
        -H "X-Bintray-Debian-Architecture: amd64" \
        -u "mmatyas:${BINTRAY_KEY}" \
        "https://api.bintray.com/content/mmatyas/deb/pool/main/p/pegasus-frontend/${filename}"
done
