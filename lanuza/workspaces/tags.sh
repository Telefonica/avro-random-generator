#!/usr/bin/env -S -u WORKSPACE bash

# gets all tags for a set of workspaces
#
# Uses the shebang line to unset env vars that may cause interference before base.inc sourcing
# https://www.gnu.org/software/coreutils/manual/html_node/env-invocation.html#env-invocation

ARG_DEFS=(
  # comma separated list of workspaces
  "[--workspace=(.+)]"
)

function run() {
  local workspaces

  if [[ -z ${WORKSPACE+x} ]]; then
    workspaces=$(workspace_list)
  else
    workspaces=$(echo ${WORKSPACE} | tr ',' '\n')
  fi

  (
    set -e
    local workspace
    while read workspace; do
      if [[ "${workspace}" != "" ]]; then
        local tags
        tags=$(workspace_tags "${workspace}")
        if [[ "${tags}" != "" ]]; then
          echo "${tags}"
        fi
      fi
    done <<< "$workspaces"
  ) | sort -u
}

source $(dirname $0)/../base.inc
