#!/usr/bin/env -S -u WORKSPACE -u TAGS -u FORMAT bash

# gets all artifacts
#
# Uses the shebang line to unset env vars that may cause interference before base.inc sourcing
# https://www.gnu.org/software/coreutils/manual/html_node/env-invocation.html#env-invocation

ARG_DEFS=(
  # comma separated list of workspaces or - to read from stdin
  "[--workspace=(.+|-)]"
  # comma separated list of tags to filter workspaces (OR)
  "[--tags=(.+)]"
  "[--format=(text|json)]"
)

function init() {
  export FORMAT="${FORMAT:-text}"
  export TAGS="${TAGS:-""}"
}

function run() {
  local workspaces 

  if [[ -z ${WORKSPACE+x} ]]; then
    # no WORKSPACE specified, use builtin API to filter by tags
    workspaces=$(workspace_list "${TAGS}")
  elif [[ "${TAGS}" == "" ]]; then
    # no TAGS. use the specified WORKSPACE
    workspaces=$(read_workspaces)
  else
    # both TAGS and WORKSPACE specified. filter manually
    local input_workspaces
    input_workspaces=$(read_workspaces)
    workspaces=$(comm -12 <(workspace_list "${TAGS}") <(echo "${input_workspaces}" | sort -u ))
  fi

  local workspace
  local output=""
  while read workspace; do
    [[ "${workspace}" != "" ]] && output="${output}$(for_workspace ${workspace})"
  done <<< "$workspaces"

  if [[ "${FORMAT}" == "json" ]]; then
    echo "${output}" | jq -s '.'
  else
    echo "${output}" | jq -sr '.[] | [ .workspace , .type , .name , .path  ] | @tsv' | column -s$'\t' -t
  fi
}

function for_workspace() {
  local workspace=$1
  workspace_manifest "${workspace}" | jq -r \
        --arg WORKSPACE "${workspace}" \
        'try .artifacts[] + {"workspace": $WORKSPACE }'
}

function read_workspaces() {
  local workspaces
  if [[ "${WORKSPACE}" == "-" ]]; then
    workspaces=$(cat)
  else
    workspaces=$(echo ${WORKSPACE} | tr ',' '\n' | sort -u)
  fi
  echo "${workspaces}"
}

source $(dirname $0)/../base.inc
