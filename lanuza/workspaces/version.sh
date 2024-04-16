#!/usr/bin/env -S -u WORKSPACE -u FORMAT bash

# returns VERSION for a set of workspaces
#
# Uses the shebang line to unset env vars that may cause interference before base.inc sourcing
# https://www.gnu.org/software/coreutils/manual/html_node/env-invocation.html#env-invocation

ARG_DEFS=(
  # comma separated list of workspaces or - to read from stdin
  "[--workspace=(.+|-)]"
  "[--format=(text|json)]"
)

function init() {
  export FORMAT="${FORMAT:-text}"
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

  local data=""
  while read workspace; do
    # add a newline between workspaces tags
    [[ "${workspace}" != "" ]] && data="${data}{\"${workspace}\": \"$(workspace_version "${workspace}")\"}"$'\n'
  done <<< "$workspaces"
  
  local output
  output=$(echo "${data}" | jq -rcs 'add | with_entries( select( .value != "" ) )')
  if [[ "${FORMAT}" == "json" ]]; then
    echo "${output}"
  else
    echo "${output}" | jq -r 'to_entries[] | [.key, .value] | @tsv' | column -s$'\t' -t
  fi
}

source $(dirname $0)/../base.inc
