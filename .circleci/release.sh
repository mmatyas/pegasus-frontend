#! /bin/bash

set -o errexit
set -o nounset
set +o xtrace # !


pushd dist

if [[ "$CIRCLE_BRANCH" != "master" ]]; then
  echo "Release uploading disabled for pull requests, uploading to Send instead"
  pushd /tmp
  curl -L -O https://github.com/mmatyas/pegasus-frontend/releases/download/alpha1/ffsend-v0.2.58-linux-x64-static
  chmod +x ffsend-v0.2.58-linux-x64-static
  popd

  for FILE in ./*; do
    timeout 5m /tmp/ffsend-v0.2.58-linux-x64-static upload $FILE || true
  done

  popd
  exit 0
fi

git config --global user.email "autodeploy@circleci.com"
git config --global user.name "Circle CI"

git init
git remote add origin https://mmatyas:${GITHUB_TOKEN}@github.com/mmatyas/pegasus-deploy-staging.git
git checkout -b continuous-${QT_TARGET}
git add ./*
git commit -m "Deployed from CircleCI"
git push -f --set-upstream origin continuous-${QT_TARGET}

popd
