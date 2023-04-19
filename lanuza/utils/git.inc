#!/usr/bin/env bash

# Get the last tag (e.g. 1.1.0). It must correspond with a version!
function git_get_last_tag() {
  local reference=$(git describe --tags --long 2> /dev/null)
  echo "${reference%-*-*}"
}

# checks if the current SHA is an annotated tag.
# Usage:
# if git_is_release_tag; then
#   echo "Is a release"
# else
#   echo "Is not a release"
# fi
function git_is_release_tag() {
  if [[ "$(git describe --exact-match  2> /dev/null)" != '' ]]; then
    return 0
  else
    return 1
  fi
}

# Get the revision: number of commits since last tag, and hash of this revision.
# For example, if describe returns 1.1.0-67-g6652da2, then revision is 67.g6652da2
function git_get_revision() {
  IFS='-' read -r -a array <<< "$(git describe --tags --long  2> /dev/null)"
  echo "${array[1]}.${array[2]}"
}

# Get the current version (the last tag or the git sha)
function git_get_version() {
  local version=$(git_get_tag)
  if [[ "${version}" == "" ]]; then
    git_get_sha
  else
    echo "${version}"
  fi
}

# gets the current git sha
function git_get_sha() {
  git rev-parse HEAD
}

# gets the current annotated tag
function git_get_tag() {
  git describe --tags --dirty --exact-match 2> /dev/null
}

# gets the current branch
function git_get_branch() {
  git rev-parse --abbrev-ref HEAD
}

# gets the current submodules in the repo
function git_get_submodules() {
  if [[ -f  "${LANUZA_ROOT}/.gitmodules" ]]; then
    git config --file "${LANUZA_ROOT}/.gitmodules" --get-regexp path | cut -d' ' -f2 || fail "Cannot get submodules from ${LANUZA_ROOT}/.gitmodules"
  fi
}
