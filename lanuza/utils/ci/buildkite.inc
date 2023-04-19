#!/usr/bin/env bash

if [[ "${BUILDKITE}" != "true" ]]; then
  return 0
fi

function git_get_branch() {
  echo "${BUILDKITE_BRANCH}"
}
