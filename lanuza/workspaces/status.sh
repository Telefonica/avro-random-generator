#!/usr/bin/env -S -u WORKSPACE -u TAGS -u FILTER bash

# checks the published status for workspaces and prints those published/missing based on the
# provided filter
#
# Uses the shebang line to unset env vars that may cause interference before base.inc sourcing
# https://www.gnu.org/software/coreutils/manual/html_node/env-invocation.html#env-invocation

ARG_DEFS=(
  "[--workspace=(.+)]"
  # comma separated list of tags to filter workspaces (OR)
  "[--tags=(.+)]"
  "[--filter=(published|missing|unknown|failed)]"
)

function init() {
  TEMP_DIR="${LANUZA_ROOT}/${LANUZA_OUTPUT_DIR}/.status.${LANUZA_BUILD_ID}"
  mkdir -p "${TEMP_DIR}"
}

function run() {
  local artifacts_output

  # generate arguments as WORKSPACE and TAGS env var as a param to the below script
  local params=("--format=text")
  [[ -z "${WORKSPACE+x}" ]] || params+=("--workspace=${WORKSPACE}")
  [[ -z "${TAGS+x}" ]] || params+=("--tags=${TAGS}")

  artifacts_output=$(./lanuza/workspaces/artifacts.sh "${params[@]}")
  # store output as artifacts array. Remove blank lines to avoid array with empty element
  mapfile -t artifacts < <(echo "${artifacts_output}" | grep -v "^$")

  # execute artifact checking for every workspace artifact, in parallel
  local artifact_line pids=()
  local file="${TEMP_DIR}/status.txt"
  for artifact_line in ${artifacts[@]+"${artifacts[@]}"}; do
    # split artifacts.sh line to paramters (split)
    # https://stackoverflow.com/questions/1469849/how-to-split-one-string-into-multiple-strings-separated-by-at-least-one-space-in
    local args=($artifact_line)
    local workspace="${args[0]}"
    local type="${args[1]}"
    local name="${args[2]}"
    local path="${args[3]}"
    # execute in parallel
    (echo "${workspace} $(artifact_exists "${type}" "${path}")") &
    pids+=($!)
  done >> ${file}

  # wait for all checks to finish
  wait "${pids[@]}"

  # group executions, as maybe there are several artifacts per workspace
  declare -A results=()
  local exit_code=0
  while read -r workspace status; do
    # priority: failed > missing > published > unknown
    if [[ -n "${results[${workspace}]+x}" ]]; then
      local prev=${results[${workspace}]}
      # we have previous result
      if [ "${status}" == "failed" ]; then
        exit_code=1
        unset results[${workspace}]
        results+=([${workspace}]=$status)
      elif [ "${status}" == "missing" ] && [ "${prev}" != "failed" ]; then
        unset results[${workspace}]
        results+=([${workspace}]=$status)
      elif [ "${status}" == "published" ] && [ "${prev}" != "failed" ] && [ "${prev}" != "missing" ]; then
        unset results[${workspace}]
        results+=([${workspace}]=$status)
      elif [ "${status}" == "unknown" ] && [ "${prev}" != "failed" ] && [ "${prev}" != "missing" ] && [ "${prev}" != "published" ]; then
        unset results[${workspace}]
        results+=([${workspace}]=$status)
      fi
    else
      results+=([${workspace}]=$status)
    fi
  done < ${file}

  # process results to print them
  local workspace
  for workspace in "${!results[@]}"; do
    local result=${results[${workspace}]}

    if [[ -n ${FILTER+x} ]]; then
      # apply the filter
      if [[ "${result}" == "$FILTER" ]]; then
        echo "${workspace}"
      fi
    else
      echo -e "${workspace}\t${result}"
    fi
  done | sort | column -s$'\t' -t

  exit ${exit_code}
}

# cleanups the cache after the main shell execution
function _status_cleanup() {
  [[ -d "$TEMP_DIR" ]] && rm -rf "$TEMP_DIR"
}

source $(dirname $0)/../base.inc
