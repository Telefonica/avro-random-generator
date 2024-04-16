#!/usr/bin/env -S -u PR -u TAGS -u FORMAT -u WS_MODE bash

# list all workspaces modified in a pull request
#
# Uses the shebang line to unset env vars that may cause interference before base.inc sourcing
# https://www.gnu.org/software/coreutils/manual/html_node/env-invocation.html#env-invocation
# but keep GITHUB_TOKEN as it's a credential and not a parameter

ARG_DEFS=(
  # GitHub PR number. Ex: 123
  "[--pr=(.+)]"
  # can be any valid git ref (branch, tag, commit SHA) pushed to GitHub. Ex: main
  "[--base=(.+)]"
  # can be any valid git ref (branch, tag, commit SHA) pushed to GitHub. Ex: feature1
  "[--head=(.+)]"
  # comma separated list of tags to filter workspaces (OR)
  "[--tags=(.+)]"
  "--github-token=(.+)"
  "[--format=(text|json)]"
  "[--ws-mode=(canonical|pr|component|standard)]"
)

function init() {
  WS_MODE=${WS_MODE:-"pr"}
  FORMAT="${FORMAT:-text}"
}

function run() {
 # declare local and use later to allow bubbling up errors in bash.
  local remote_files

  # check if PR is Defined
  if [[ -z ${PR+x} ]]; then
    # no PR specified, if no BASE and HEAD are specified, fail
    if [[ -z ${BASE+x} || -z ${HEAD+x} ]]; then
      fail "Either --pr or --base and --head must be provided"
    fi
    remote_files=$(github_get_branches_files "${BASE}" "${HEAD}")
  else
    # PR specified. use the specified PR
    # if PR and BASE or HEAD are specified, fail
    if [[ -n ${BASE+x} || -n ${HEAD+x} ]]; then
      fail "Either --pr or --base and --head must be provided"
    fi
    remote_files=$(github_get_pr_files "${PR}")
  fi

  # GitHub, when a submodule is updated, only returns the submodule folder modified
  # When a submodule is updated, get all the current modified folder files and add to the
  # set of modified files.
  # TODO: improve this by getting only modified files between the SHA's that have been updated
  local submodules
  submodules=$(git_get_submodules)

  # keep in the main shell with process substitution to allow writting vars from main shell
  local modified_file
  while read modified_file; do
    # TODO: avoid this loop by searching in the $submodules string
    local submodule
    while read submodule; do
      if [[ "$modified_file" == "${submodule}" ]]; then
        debug "PR modified submodule \"${submodule}\""
        local submodule_files=$(fs_list "${submodule}")
        # remove the submodule from the list.
        # Use # as sed separator as submodule can be a path with /, the default separator
        remote_files=$(echo "${remote_files}" | sed "s#${submodule}##" )
        # add all the submodule files to the list
        remote_files="${remote_files}"$'\n'"${submodule_files}"
      fi
    done <<< "${submodules}"
  done <<< "${remote_files}"

  # remove possible empty lines and sort, to prepare for later comparison
  remote_files=$(echo "${remote_files}" | sed '/^$/d' | sort)

  # get and echo later to bubble up errors up
  local workspaces
  workspaces=$(workspace_list "${TAGS:-""}")

  # keep in the main shell with process substitution to allow writting vars from main shell
  local workspace
  local output=""
  while read workspace; do
    [[ "${workspace}" == "" ]] && continue
    local ws_files modified_files count

    # GitHub does not know about symlinks, so we have to translate our
    # files to its canonical name to check if they have been modified (canonical)
    # and also get files without honoring .ignore files for that specific component
    ws_files=$(workspace_files "${workspace}" "${WS_MODE}")

    # make the intersection for both ordered sets to get the modified files
    modified_files=$(comm -12 <(echo "${ws_files}") <(echo "${remote_files}"))

    if [[ "${modified_files}" != "" ]]; then
      # echo "" | wc -l => returs 1, so
      # use xargs to trim wc output
      # https://stackoverflow.com/questions/369758/how-to-trim-whitespace-from-a-bash-variable
      count=$(echo "${modified_files}" | wc -l | xargs )
    else
      count="0"
    fi

    echoerr "Found ${count} modified files in workspace \"${workspace}\":" ${modified_files}

    if [[ ${count} != "0" ]]; then
      output="${output}${workspace}"$'\n'
    fi

  done <<< "${workspaces}"

  if [[ "${FORMAT}" == "json" ]]; then
    # sort and remove empty lines and convert to json array
    echo "${output}" | sort -u | sed '/^[[:space:]]*$/d' | jq -cnR '[inputs | select(length>0)]'
  else
    echo "${output}" | sort -u | sed '/^[[:space:]]*$/d'
  fi
}

source $(dirname $0)/../base.inc
