#!/usr/bin/env bash

# Calculate the absolute path of a file
# TODO: use one of the /utils/bin/canonical.sh functions
function path_absolute() {
  # $1 : relative filename
  [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

# put our bin files in the PATH
# https://stackoverflow.com/questions/4774054/reliable-way-for-a-bash-script-to-get-the-full-path-to-itself/4774063
PATHO="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/bin"
export PATH="$PATH:$PATHO"
