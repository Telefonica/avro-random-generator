#!/usr/bin/env -S -u WORKSPACE -u TYPE -u COMPRESS bash

# prints all outputs for a workspace
#
# Uses the shebang line to unset env vars that may cause interference before base.inc sourcing
# https://www.gnu.org/software/coreutils/manual/html_node/env-invocation.html#env-invocation

ARG_DEFS=(
  # workspaces
  "[--workspace=(.+)]"
  # the artifact type files to return.
  "[--type=(output|.+)]"
  # compress the involved files and return the final filename
  # not setting this will output the files.
  "[--compress=(.+)]"
)

function run() {
  local artifacts workspaces files
  if [[ -z ${WORKSPACE+x} ]]; then
    # no WORKSPACE specified
    workspaces=$(workspace_list)
  else
    workspaces=$(echo ${WORKSPACE} | tr ',' '\n' | sort -u)
  fi

  files=$((
    local workspace
    while read workspace; do
      for_workspace "${workspace}"
    done <<< "$workspaces"
  ) | sort -u)

  if [[ "${files}" == "" ]]; then
    return
  fi

  if [[ -z ${COMPRESS+x} ]]; then
    echo "$files"
  else
    local dest="${LANUZA_OUTPUT_DIR}/${COMPRESS}.tar.gz"
    echo "${files}" | tr '\n' '\0' | xargs -0 tar -czf "${dest}"
    echo "${dest}"
  fi
}

function for_workspace() {
  local workspace="${1}"
  local paths workspace_path
  if [[ -z ${TYPE+x} ]]; then
    local artifacts
    workspace_path=$(workspace_path "${workspace}")
    artifacts=$(workspace_manifest "${workspace}" | jq -r 'try .artifacts[] | .path')
    paths=$(printf "%s\n%s" "${workspace_path}/${LANUZA_OUTPUT_DIR}/**/*" "${artifacts}")
  elif [[ "${TYPE}" == "output" ]]; then
    workspace_path=$(workspace_path "${workspace}")
    paths="${workspace_path}/${LANUZA_OUTPUT_DIR}/**/*"
  else
    paths=$(workspace_manifest "${workspace}" | jq -r --arg TYPE ${TYPE} 'try .artifacts[] | select(.type == $TYPE) | .path')
  fi

  local path
  while read path; do
    [[ "${path}" != "" ]] && fs_list -u -g "${path}" || true
  done < <(echo "${paths}")
}

source $(dirname $0)/../base.inc
