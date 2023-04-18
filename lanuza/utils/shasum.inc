#!/usr/bin/env bash

# Get the current version (the last tag or the git sha)
# TODO: use only git to perform sha calculation
function shasum_get_version() {
  local version=$(git_get_tag)
  if [[ "${version}" == "" ]]; then
    shasum_get_sha
  else
    echo "${version}"
  fi
}

# Get the current version (based on the file sha)
function shasum_get_sha() {
  if workspace_enabled; then
    workspace_get_sha
  else
    _shasum_dir
  fi
}

# Calculates the SHA1 sum for files in the current dir. honors .ignore
function _shasum_dir() {
  # get the files, including symlinks. Use a null char to separete filenames instead of \n, to avoid
  # problems with whitespaces in the filenames
  fs_list --null | _shasum_files
}

# Calculates the sha for a set of files. Needs files separated with nulls
function _shasum_files() {
  #1 call out shasum wrapper, using null (-0) as a separator
  #2 pick the first column, the sha
  #3 sort and drop duplicated shas, because we dont trust files to have always the same ordering.
  #  it's more efficient doing here because shas are potentially shorter.
  #4 Remove whitespaces in the ordered sha, making a big string
  #5 calculate the sha for all the shas. This is why ordering is important
  #6 and take the first column with the final sha
  xargs -0 shasum.sh | cut -d" " -f1 | sort -u | tr -cd '0-9a-fA-F' | shasum.sh - | cut -d" " -f1
}
