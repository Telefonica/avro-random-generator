#!/usr/bin/env -S -u PR -u TAGS bash

# list all workspaces modified in a pull request
#
# Uses the shebang line to unset env vars that may cause interference before base.inc sourcing
# https://www.gnu.org/software/coreutils/manual/html_node/env-invocation.html#env-invocation
# but keep GITHUB_TOKEN as it's a credential and not a parameter

ARG_DEFS=(
  "--pr=(.+)"
  # comma separated list of tags to filter workspaces (OR)
  "[--tags=(.+)]"
  "--github-token=(.+)"
)

function run() {
 # declare local and use later to allow bubbling up errors in bash.
  local pr_files
  echoerr "Getting files for PR ${PR}"
  pr_files=$(github_get_pr_files "${PR}")

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
        pr_files=$(echo "${pr_files}" | sed "s#${submodule}##" )
        # add all the submodule files to the list
        pr_files="${pr_files}"$'\n'"${submodule_files}"
      fi
    done <<< "${submodules}"
  done <<< "${pr_files}"

  # remove possible empty lines and sort, to prepare for later comparison
  pr_files=$(echo "${pr_files}" | sed '/^$/d' | sort)

  # get and echo later to bubble up errors up
  local workspaces
  workspaces=$(workspace_list "${TAGS:-""}")

  # keep in the main shell with process substitution to allow writting vars from main shell
  local workspace
  while read workspace; do
    [[ "${workspace}" == "" ]] && continue
    local ws_files modified_files count

    # GitHub does not know about symlinks, so we have to translate our
    # files to its canonical name to check if they have been modified (canonical)
    # and also get files without honoring .ignore files for that specific component
    ws_files=$(workspace_files "${workspace}" "pr")

    # make the intersection for both ordered sets to get the modified files
    modified_files=$(comm -12 <(echo "${ws_files}") <(echo "${pr_files}"))

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
      echo "${workspace}"
    fi

  done <<< "${workspaces}"
}

source $(dirname $0)/../base.inc
