#!/usr/bin/env bash

if [[ "${GITHUB_ACTIONS}" != "true" ]]; then
  return 0
fi

function git_get_branch() {
  echo "${GITHUB_HEAD_REF:-${GITHUB_REF#refs/heads/}}"
}
