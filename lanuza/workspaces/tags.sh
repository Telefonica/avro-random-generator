#!/usr/bin/env -S -u WORKSPACE -u FORMAT bash

# gets all tags for a set of workspaces
#
# Uses the shebang line to unset env vars that may cause interference before base.inc sourcing
# https://www.gnu.org/software/coreutils/manual/html_node/env-invocation.html#env-invocation

ARG_DEFS=(
  # comma separated list of workspaces or - to read from stdin
  "[--workspace=(.+|-)]"
  "[--format=(text|json)]"
)

function init() {
  FORMAT="${FORMAT:-text}"
}

function run() {
  local workspaces

  if [[ -z ${WORKSPACE+x} ]]; then
    workspaces=$(workspace_list)
  elif [[ "${WORKSPACE}" == "-" ]]; then
    workspaces=$(cat)
  else
    workspaces=$(echo ${WORKSPACE} | tr ',' '\n')
  fi

  local output=""
  while read workspace; do
    # add a newline between workspaces tags
    [[ "${workspace}" != "" ]] && output="${output}$(workspace_tags "${workspace}")"$'\n'
  done <<< "$workspaces"

  if [[ "${FORMAT}" == "json" ]]; then
    # sort and remove empty lines and convert to json array
    echo "${output}" | sort -u | sed '/^[[:space:]]*$/d' | jq -cnR '[inputs | select(length>0)]'
  else
    echo "${output}" | sort -u | sed '/^[[:space:]]*$/d'
  fi
}

source $(dirname $0)/../base.inc
