#!/usr/bin/env bash

# Unify how sorting (sort pipes) must be done in different systems with different locales, to
# avoid different shasums
# https://unix.stackexchange.com/questions/29681/issues-of-using-sort-and-comm
export LC_COLLATE=C

# all workspace projects paths
declare -A lanuza_ws_projects=()
declare -A lanuza_ws_tags=()

LANUZA_WORKSPACE_CACHE_DIR="${LANUZA_ROOT}/${LANUZA_OUTPUT_DIR}/.workspace.${LANUZA_BUILD_ID}"

# Initializes the workspaces variables for subsquent commands
# private
function _workspace_init() {
  # compute only if an empty array
  if [[ "${#lanuza_ws_projects[@]}" == 0 ]]; then
    function get_workspaces() {
      (
        set -euo pipefail
        cd "${LANUZA_ROOT}"
        debug "Finding all workspaces..."

        local workspace
        fs_list --no-ignore-dot -g 'workspace.lanuza' | canonical.sh | sort -u | while read -r workspace; do
          local path name
          path=$(dirname "${workspace}")
          name=$(jq -r -e '.name' "${workspace}") || fail "Cannot get mandatory \"name\" property for ${workspace}"
          debug "  //${name}: ${path}"
          echo "${name} ${path}"
        done
      )
    }

    local workspaces
    workspaces=$(_workspace_cache_get "workspaces" "get_workspaces") || fail "Cannot get workspaces"

    # load to our internal associative array
    local line
    while read line; do
      local args=($line)
      local name="${args[0]}"
      local path="${args[1]}"
      lanuza_ws_projects[${name}]="${path}"
    done <<< "${workspaces}"
  fi
}

# Initializes the workspaces tags for subsquent commands
# private
function _workspace_init_tags() {
  if [[ "${#lanuza_ws_tags[@]}" == 0 ]]; then
    function get_tags() {
      (
        set -euo pipefail
        cd "${LANUZA_ROOT}"
        debug "Finding all tags..."

        for name in "${!lanuza_ws_projects[@]}"; do
          local path=${lanuza_ws_projects[${name}]}

          local btag=""
          if [[ -f "${path}/lanuza/pipelines/publish.sh" ]]; then
            btag="lanuza:pipelines:publish"
          fi

          local ptag=""
          if [[ -f "${path}/lanuza/pipelines/build.sh" ]]; then
            ptag="lanuza:pipelines:build"
          fi

          local tags
          tags=$(jq -r  \
            --arg btag "${btag}" \
            --arg ptag "${ptag}" \
            '[ $btag, $ptag ] + try .tags | [ .[] |  select(length > 0) ] | sort | join(",")' \
            "${path}/workspace.lanuza"
          ) || return 1

          debug "  //${name}: ${tags}"
          echo "${name} ${tags}"
        done
      )
    }

    local tags
    tags=$(_workspace_cache_get "tags" "get_tags") || fail "Cannot get tags"

    # load to our internal associative array
    local line
    while read line; do
      local args=($line)
      local name="${args[0]}"
      local tags="${args[1]:-""}" # maybe there are no tags
      lanuza_ws_tags[${name}]="${tags}"
    done <<< "${tags}"
  fi
}

# print the names of all the available projects with a workspace
function workspace_list() {
  local tagString="${1:-""}"
  # make the intersection for both ordered sets to get the modified files
  (
    set -euo pipefail
    cd "${LANUZA_ROOT}"
    _workspace_init
    if [[ "$tagString" != "" ]]; then
      _workspace_init_tags
      local tags=$(echo $tagString | tr "," "\n" | sort -u )
      local name
      for name in "${!lanuza_ws_projects[@]}"; do
        local matches
        local ws_tags=$(echo ${lanuza_ws_tags[$name]} | tr "," "\n")

        matches=$(comm -12 <(echo "${tags}") <(echo "${ws_tags}"))
        if [[ "${matches}" != "" ]]; then
          printf '%s\n' "${name}"
        fi
      done
    else
      for name in "${!lanuza_ws_projects[@]}"; do
        printf '%s\n' "${name}"
      done
    fi
  ) | sort -u
}

# gets the worspace sha
function workspace_get_sha() {
  local name
  local mode
  name=${1:-$(workspace_name)}
  mode=${2:-"standard"}
  (
    set -euo pipefail
    cd "${LANUZA_ROOT}"

    local sha

    _workspace_init
    _workspace_assert_workspace_exists ${name}

    # TODO: Performance improvement
    # getting files for a workspace calculates shas one and over again for the //deps,
    # because the files are inlined. Calculating the //dep sha and caching will improve
    # performace
    sha=$(workspace_files ${name} ${mode} | tr '\n' '\0' | _shasum_files)
    echo "${sha}"
  )
}

# returns the path for a workspace
function workspace_path() {
  local name manifest path local_path
  name=${1:-$(workspace_name)}
  _workspace_init
  _workspace_assert_workspace_exists ${name}
  echo "${lanuza_ws_projects[${name}]}"
}

# Runs a command in all workspaces
function workspace_run() {
  (
    set -euo pipefail
    cd "${LANUZA_ROOT}"

    local pipeline=$1

    _workspace_init
    local key
    for key in "${!lanuza_ws_projects[@]}"; do
      local path=${lanuza_ws_projects[${key}]}
      local full_path="${path}/lanuza/pipelines/${pipeline}.sh"
      if [[ -f "$full_path" ]]; then
        $full_path
      else
        echoerr "Not found \"$full_path\". Skipping"
      fi
    done
  )
}

# Runs a command in a specific workspace
function workspace_exec() {
  local name
  name=${1:-$(workspace_name)}
  local pipeline=$2
  local path
  (
    set -euo pipefail
    cd "${LANUZA_ROOT}"

    _workspace_init
    _workspace_assert_workspace_exists ${name}
    path=${lanuza_ws_projects[${name}]}

    local full_path="${path}/lanuza/pipelines/${pipeline}.sh"
    if [[ -f "$full_path" ]]; then
      $full_path
    else
      fail "\"$full_path\" does not exist for workspace ${name}"
    fi
  )
}

# echoes the VERSION env var declared for a publish pipeline in a workspace
function workspace_version() {
  local name path 
  name=${1:-$(workspace_name)}
  _workspace_init
  _workspace_assert_workspace_exists ${name}
  path=${lanuza_ws_projects[${name}]}
  (
    set -e
    cd "${path}"
    if [[ -f "./lanuza/pipelines/publish.sh" ]]; then
      _workspace_load_publish_init
    fi
    echo "${VERSION:-""}"
  )
}

# echoes the current namespace manifest with env from publish pipeline interpolated
function workspace_manifest() {
  local name manifest path
  name=${1:-$(workspace_name)}
  _workspace_init
  _workspace_assert_workspace_exists ${name}
  path=${lanuza_ws_projects[${name}]}
  (
    set -e
    cd "${path}"
    if [[ -f "./lanuza/pipelines/publish.sh" ]]; then
      _workspace_load_publish_init
    fi
    # load manifest and interpolate the env vars in the current shell
    # TODO: inherited values will be interpolated also
    manifest=$(cat "./workspace.lanuza")
    echo $(envsubst <<< "${manifest}")
  )
}


# try to load the publish pipeline to get its env vars
function _workspace_load_publish_init() {
  debug "Loading \"${name}\" publish pipeline init function"
  # remove any init function that may have been defined because this method
  # is called from another lanuza script
  unset -f init
  # remove the lanuza base source to only get function definitions
  # and load the pipeline in the subshell
  source <(cat ./lanuza/pipelines/publish.sh | sed '/source $(dirname $0)\/..\/base.inc/d' | sed '/source "$(dirname $0)"\/..\/base.inc/d' | sed '/source "$(dirname "$0")"\/..\/base.inc/d')
  # call the init function to get all en vars in our shell session
  if isFunction "init"; then
    # list of env vars to clean to avoid side-effects when calling from
    # componentes
    # TODO: This list should be created on-demand by parsing init contents and see what
    # variables are getting the default or maybe exposing a configuration env_var per pipeline
    local clean=("VERSION" "TAG")
    local env_var
    for env_var in "${clean[@]}"; do
      unset "${env_var}"
    done
    init
  fi
}

# echoes the current namespace tags
function workspace_tags() {
  local name
  name=${1:-$(workspace_name)}
  _workspace_init
  _workspace_assert_workspace_exists ${name}
  _workspace_init_tags

  if [[ "${lanuza_ws_tags[${name}]}" != "" ]]; then
    echo ${lanuza_ws_tags[$name]} | tr "," "\n"
  fi
}

# get the workspace project name in a path
function workspace_name() {
  local path=${1:-"."}

  _workspace_assert_manifest "${path}"

  local workspace_file="${path}/workspace.lanuza"

  jq -r -e '.name' "${workspace_file}" || fail "Cannot get mandatory \"name\" property for ${workspace_file}"
}

function _workspace_assert_manifest() {
  local path=${1:-"."}

  local workspace_file="${path}/workspace.lanuza"
  [[ -f "${workspace_file}" ]] || fail "Missing project manifest \"${workspace_file}\""
}

function _workspace_assert_workspace_exists() {
  local name=$1
  [[ -n "${lanuza_ws_projects[${name}]+x}" ]] || fail "No \"${name}\" workspace found"
}

# Gets all files for a workspace
# can return canonical paths, i.e, real paths for files that are symlinks or belong to a
# symlinked folder. This operation is heavy, and can have a great penalty in performance
# usage:
#  workspace_files "ws_name"
#  workspace_files "ws_name" "canonical" # to return canonical files
#  workspace_files "ws_name" "pr" # to return canonical files for a workspace without ignoring the workspace files
#  workspace_files "ws_name" "component" # to return canonical files for a workspace ignoring component dependencies
#  workspace_files "ws_name" "standard" # to return files ignoring workpace ignore files
# The third parameter is internal, a recursive call check
function workspace_files() {
  # we first declare local, and then get defaults.
  # this way, when can safely use default getters with propagated errors
  local name circular mode
  # debug "workspace_files $@"
  name=${1:-$(workspace_name)}
  mode=${2:-"standard"}
  circular=${3:-""} # simple recursive call stack

  local path list_path
  local stack="${circular} -> //${name}"
  # check if our manual simple call stack contains the current workspace
  # to detect circular dependencies
  if [[ ${circular} == *"//${name} ->"* ]]; then
    fail "Circular Dependency found in deps!" "${stack}"
  fi

  function get_files() {
    (
      set -euo pipefail
      cd "${LANUZA_ROOT}"

      _workspace_init
      _workspace_assert_workspace_exists ${name}

      path=${lanuza_ws_projects[${name}]}

      # if path == '.' we are in the root dir, and we need to get all files without telling
      # rg that the path is ".", because it will prefix all results with ./
      if [[ "${path}" == "." ]]; then
        list_path=""
      else
        list_path="${path}"
      fi

      debug " ${stack}"
      if [[ "${mode}" == "pr" ]]; then
        # In "pr" mode, we get all workspace files not using the .ignore files in a canonical (resolving symlinks) format,
        # to allow comparing with the modified files in the PR
        # This way we can get all component files that maybe be ignored (test, etc... ) by users
        # and allow CI processes to run testing because it may exists test modified files that are ignored to
        # compute the SHA version in a published artifact, but must not be ignored to run tests
        fs_list --no-ignore-dot ${list_path} | canonical.sh
        # switch to canonical mode to process deps, where we dont care about test modified files
        mode="canonical"
      elif [[ "${mode}" == "component" ]]; then
        # In "component" mode, we get all workspace files not using the .ignore files
        # This way we can get all component files that maybe be ignored (test, etc... ) by users
        # and compute things like the whole component sha (including test files)
        fs_list --no-ignore-dot ${list_path}
        mode="standard"
      elif [[ "${mode}" == "canonical" ]]; then
        # In "canonical" mode, we get all workspace files resolving symlinks
        fs_list ${list_path} | canonical.sh
        mode="canonical"
      else
        # In "standard" mode, we get all workspace files honoring the .***ignore files
        fs_list ${list_path}
        mode="standard"
      fi

      # use process substitution here to stay in the main shell
      # https://stackoverflow.com/questions/33901425/does-a-recursive-bash-function-lose-its-ability-to-return-after-descending
      local dep
      local globs=()
      while IFS= read -r -d '' dep; do
        if [[ ${dep:0:2} == "//" ]]; then
          # the dependency starts with //, we have a project dependency.
          local project=${dep:2}
          workspace_files "${project}" "${mode}" "${stack}" || return 1
        else
          # we have file dependencies. We support globs, so lets delegate on rg to get the involved files
          globs+=("-g" "${dep}")
        fi
      # convert to null character joins
      done < <(jq -r 'try .deps[]' "${path}"/workspace.lanuza | tr '\n' '\0')

      if [[ ${#globs[@]} -ne 0 ]]; then
        # deactivate .ignore files, to allow referencing any file
        debug " ${stack} -> ${globs[@]}"
        if [[ "${mode}" == "canonical" ]]; then
          fs_list -u --hidden "${globs[@]}" | canonical.sh || fail "Invalid glob \"${globs[@]}\" in \"${name}\" workspace deps"
        else
          fs_list -u --hidden "${globs[@]}" || fail "Invalid glob \"${globs[@]}\" in \"${name}\" workspace deps"
        fi
      fi
    ) | sort -u # return sorted and without dupes
  }

  _workspace_cache_get "files.${mode}.${name}" "get_files"
}

# Checks if the workspace is enabled
# Usage:
#  if workspace_enabled; then ... fi
function workspace_enabled() {
  if [[ -f "workspace.lanuza" ]]; then
    return 0
  else
    return 1
  fi
}

# cleanups the cache after the main shell execution
function _workspace_cache_cleanup() {
  [[ -d "$LANUZA_WORKSPACE_CACHE_DIR" ]] && rm -rf "$LANUZA_WORKSPACE_CACHE_DIR"
}

# getOrFetch cache. returns a file from the cache, or calls the
# provided function to get it and store in the cache
# usage:
#  _workspace_cache_get "key" "values_getter_fn"
function _workspace_cache_get() {
  set -e
  local key="${1}"
  local getter="${2}"

  local file="${LANUZA_WORKSPACE_CACHE_DIR}/${key}"
  if ! [[ -f "$file" ]]; then
    mkdir -p "${LANUZA_WORKSPACE_CACHE_DIR}"
    # to avoid creating the file when the $getter fails
    local output
    output=$($getter) || return 1
    echo "$output" > "${file}"
  fi
  cat "${file}"
}
