#!/usr/bin/env bash

# https://stackoverflow.com/questions/24318927/bash-regex-to-match-semantic-version-number
SEMVER_REGEX="^(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)(\\-[0-9A-Za-z-]+(\\.[0-9A-Za-z-]+)*)?(\\+[0-9A-Za-z-]+(\\.[0-9A-Za-z-]+)*)?$"

# Checks if the provided version is a release valid semver
# if version_is_release "1.2.3"; then
#   echo "Is a release"
# else
#   echo "Is not a release"
# fi
function version_is_release() {
  local version=$1

  if [[ "$version" =~ $SEMVER_REGEX ]]; then
    return 0
  else
    return 1
  fi
}

# Gets the current tag for artifacts. 
# export TAG="$(version_get_tag ${VERSION})"
function version_get_tag() {
  local version=$1
  local tag=$(git_get_tag)
  if [[ "${tag}" == "" ]]; then
    echo $version
  else
    echo $tag
  fi  
}
