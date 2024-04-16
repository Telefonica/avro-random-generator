#!/usr/bin/env bash

# export current user variables to allow using in docker containers or docker-compose files
# We cannot use $UID/$GID as they are bash variables, and are not standard environment variables.
# that means they are not available in every shell, and are not read by env var substitution utilities,
# like envsubs or docker-comopose.yml parsers for [.env|env_file]...
#
# The usage is as follows:
# - When this file is sourced in the host, in a pipelines/xxxx.sh, it will store your current user
# - When making a `docker compose run` inside a pipelines/xxxxx.sh, these vars it will be automatically
#   injected into the container (see utils/docker.inc#_docker_compose_run decoration). This way,
#   we make the user command executed inside the container (usually scripts/xxxxx.sh) aware of the host user
#   with no extra coding effort
# The container executing the script can use `gosu` and these env vars to create and execute the command/script
# as the host user. This way, all files created by the containers will belong to the host user and not root,
# and will be accessible from host processes like CI agents and proccesess.
# See docker-entrypoint.sh and Dockerfile in lanuza repo for an example of this behabiour
export DOCKER_UID=${DOCKER_UID:-$(id -u)}
export DOCKER_GID=${DOCKER_GID:-$(id -g)}

# Configure docker to use the enhaced build features
export DOCKER_BUILDKIT="${DOCKER_BUILDKIT:-1}"

# Provide CI default configuration, as an human may not be see this
if [ -n "${CI}" ]; then
  # Disable "Use 'docker scan' to run Snyk tests..." message
  export DOCKER_SCAN_SUGGEST=${DOCKER_SCAN_SUGGEST:-false}
  # Make the output friendly with log files
  export BUILDKIT_PROGRESS=${BUILDKIT_PROGRESS:-plain}
else
  export BUILDKIT_PROGRESS=${BUILDKIT_PROGRESS:-auto}
fi

# lanuza builder name to use when building via docker compose build
export LANUZA_BUILDX_DRIVER=${LANUZA_BUILDX_DRIVER:-lanuza}

# lanuza builder image to use when building via docker compose build
export LANUZA_BUILDX_DRIVER_IMAGE=${LANUZA_BUILDX_DRIVER_IMAGE:-moby/buildkit:v0.11.6}

# flag to know when should we autocleanup docker compose
LANUZA_DOCKER_COMPOSE__CLEANUP=0

# Checks if an image exists locally
function docker_image_exists() {
  local image="$1"
  _docker image inspect "${image}" &> /dev/null
}

# Runs docker ps -a filtered by the current project name
function _docker_ps_by_project() {
  docker ps -a \
    --filter "label=com.docker.compose.project=${COMPOSE_PROJECT_NAME}" \
    "${@}"
}

# Decorate docker command
function docker() {
  # docker compose  ...
  if [ $1 == "compose" ]; then
    LANUZA_DOCKER_COMPOSE__CLEANUP=1
      # docker compose run ...
    if [ $2 == "run" ]; then
      _docker_compose_run "${@:2}"
    # docker compose build ...
    elif [ $2 == "build" ]; then
      _docker_compose_build "${@:2}"
    else
      command docker "$@"
    fi
  else
    command docker "$@"
  fi
}

# executes docker  command
function _docker() {
  debug "docker $@"
  command docker "$@"
}

# executes docker compose command
function _docker_compose() {
  debug "docker compose $@"
  command docker compose "$@"
}

# docker compose build handler
# decorates the command to add lanuza build features like
# - VERSION handling
# - automatic host docker user propagation
# - caching
function _docker_compose_build() {
  # get services affected by build
  local services=()
  if [ -z ${2+x} ]; then
    # use all docker compose services stated in the compose files
    services+=( $(_docker compose config --services) )
  else
    # we have an specific "docker compose build <service>". Use that service in our cache
    services+=($2)
  fi

  # Removing services that are not buildable
  # "docker compose config" allows to get all services using the exported COMPOSE_FILE files
  # Using all (*) profiles to get all services, avoiding possible filtering in userland
  local config
  config=$(COMPOSE_PROFILES="*" _docker compose config --no-normalize --format json)
  for index in ${!services[@]}; do
    local service=${services[$index]}
    local exists=$(echo "${config}" | jq --arg service "${service}" 'try(.services[$service] | has("build"))')
    if [[ "${exists}" == "false" ]]; then
      debug "${service} is not buildable, skipping"
      unset services[$index]
    fi
  done

  # compute image caches when enabled
  declare -A build_cache_from
  declare -A build_cache_to

  if [[ ! -z ${LANUZA_INLINE_CACHE_REPO+x} ]]; then
    local dest_image_prefix
    if workspace_enabled; then
      dest_image_prefix="${LANUZA_INLINE_CACHE_REPO}/$(workspace_name)"
    else
      dest_image_prefix="${LANUZA_INLINE_CACHE_REPO}/${LANUZA_PROJECT}"
    fi

    for service in ${services[@]}; do
      # store our target branches in array
      IFS=':' read -r -a branches <<< "${LANUZA_INLINE_CACHE_BRANCHES:-master}"
      # generate the cache-from for the branches
      local dest_image="${dest_image_prefix}_${service}"
      for branch in ${branches[@]}; do
        local tag=$(_docker_compose_normalize_tag "$branch")
        build_cache_from[${service}]+="        - ${dest_image}:${tag}"$'\n'
      done

      # generate the cache_to when the current branch is configured and when we are in CI
      # to avoid pushing from developers machines
      local branch
      branch=$(git_get_branch)
      if [[ ! -z ${CI+x} ]] && [[ " ${branches[*]} " =~ " ${branch} " ]]; then
        local tag_to=$(_docker_compose_normalize_tag "$branch")
        build_cache_to[${service}]+="        - ${dest_image}:${tag_to}"$'\n'
      fi
    done
  fi

  # Only build it there are buildable services
  if [[ ${#services[@]} -ne 0 ]]; then
    # build automatically with VERSION
    # TODO: VERSION should be 0.0.0-snapshot, but this is breaking change
    set -- "build" "--build-arg" "VERSION=${VERSION:-snapshot}" "${@:2}"
    set -- "build" "--build-arg" "DOCKER_UID=${DOCKER_UID}" "${@:2}"
    set -- "build" "--build-arg" "DOCKER_GID=${DOCKER_GID}" "${@:2}"

    declare -A build_labels
    local date version revision

    date="$(date -u +"%Y-%m-%dT%H:%M:%SZ")"
    version="$(git_get_version)"
    revision="$(git_get_sha)"

    # The final yaml to compute
    local yaml=$'\n'"services:"$'\n'
    for service in ${services[@]}; do
      # set standard labels https://github.com/opencontainers/image-spec/blob/main/annotations.md
      build_labels[${service}]+="        org.opencontainers.image.created: \"$date\""$'\n'
      build_labels[${service}]+="        org.opencontainers.image.version: \"$version\""$'\n'
      build_labels[${service}]+="        org.opencontainers.image.revision: \"$revision\""$'\n'

      yaml+="  ${service}:"$'\n'
      yaml+="    build:"$'\n'
      yaml+="      labels:"$'\n'
      yaml+="${build_labels[${service}]}"
      if [[ -v "build_cache_from[${service}]" ]]; then
        yaml+="      cache_from:"$'\n'
        yaml+="${build_cache_from[${service}]}"
      fi
      if [[ -v "build_cache_to[${service}]" ]]; then
        yaml+="      cache_to:"$'\n'
        yaml+="${build_cache_to[${service}]}"
      fi
    done

    # Include new yaml in the docker-compose files
    temp_compose=$(: ${TMPDIR:=/tmp}; mktemp "${TMPDIR%/}/docker-compose.labels.$LANUZA_BUILD_ID.XXXXXXXXX")

    # To cleanup at exit
    temp_files+=( $temp_compose )

    debug "Using the following docker compose file to customize build"
    debug "$yaml"
    echo "$yaml" > $temp_compose

    IFS="${COMPOSE_PATH_SEPARATOR}" read -r -a compose_files <<< "${COMPOSE_FILE:-docker-compose.yml}${COMPOSE_PATH_SEPARATOR}${temp_compose}"
    # reverse order to keep the original COMPOSE_FILE order
    for ((i=${#compose_files[@]}-1; i>=0; i--)); do
      set -- "-f" "${compose_files[$i]}" "${@:1}"
    done
    _docker_setup_builder
  fi
  _docker_compose "$@"
}

# docker compose run handler
function _docker_compose_run() {
  LANUZA_SAVE_CONTAINER_LOGS="${LANUZA_SAVE_CONTAINER_LOGS:-"on-error"}"

  # Allow the host user to run commands in docker
  # propagate internal env variables to the run commands
  set -- "run" "-e" "DOCKER_UID=$DOCKER_UID" "${@:2}"
  set -- "run" "-e" "DOCKER_GID=$DOCKER_GID" "${@:2}"
  set -- "run" "-e" "LANUZA_BUILD_ID=$LANUZA_BUILD_ID" "${@:2}"
  set -- "run" "-e" "DEBUG" "${@:2}"

  set +e
  _docker_compose "$@"
  exitcode=$?
  set -e

  # Get list of failed containers
  containers=()
  while read -r container ; do
    [[ -n "$container" ]] && containers+=("$container")
  done <<< "$(_docker_ps_by_project -q)"

  failed_containers=()
  if [[ 0 != "${#containers[@]}" ]] ; then
    while read -r container ; do
      [[ -n "$container" ]] && failed_containers+=("$container")
    done <<< "$(docker inspect -f '{{if ne 0 .State.ExitCode}}{{.Name}}.{{.State.ExitCode}}{{ end }}' "${containers[@]}")"
  fi

  if [[ 0 != "${#failed_containers[@]}" ]] ; then
    echo "Some containers had non-zero exit codes"
    _docker_ps_by_project \
      --format 'table {{.Label "com.docker.compose.service"}}\t{{.Command}}\t{{ .Status }}' \
      --no-trunc
  fi

  if [[ "${LANUZA_SAVE_CONTAINER_LOGS}" =~ ^(true|on|1|always)$ ]]; then
    _docker_compose_save_logs
  fi

  if [[ "${LANUZA_SAVE_CONTAINER_LOGS}" =~ ^(on-error)$ ]] && [[ 0 != "${#failed_containers[@]}" ]]; then
    _docker_compose_save_logs
  fi

  if [[ $exitcode -ne 0 ]] ; then
    exit $exitcode
  fi
}

# automatic cleanup function for docker commands
function _docker_compose_cleanup() {
  if [ "$LANUZA_DOCKER_COMPOSE__CLEANUP" -eq 1 ]; then
    # XXX: in docker compose v2.15.1, using --rmi local deletes the built images with tags
    # when they have been created with a `docker compose build`. but does not delete them
    # when thay have been created with `docker compose up`. So deleting them will break a
    # common use case like "use a pipeline for building and use it from another pipeline"
    _docker compose down -v --remove-orphans || true
    # bash < 4.4 support in the "in" condition
    # https://stackoverflow.com/questions/7577052/bash-empty-array-expansion-with-set-u
    for temp_file in ${temp_files[@]+"${temp_files[@]}"}; do
      rm -fR $temp_file || true
    done
  fi
}

# Normalize tag names to be used as docker registries tags.
# A tag name must be valid ASCII and may contain lowercase and uppercase letters, digits, underscores, periods and dashes.
# A tag name may not start with a period or a dash and may contain a maximum of 128 characters.
# Replace offending chars with - and trim
function _docker_compose_normalize_tag() {
  local tag=$1
  echo "${tag}" | sed 's/[^a-zA-Z0-9_\-\.]/-/g' | cut -c1-128
}

# setup a buildx builder instance
# TODO: restore the previous instance on cleanup
function _docker_setup_builder() {
  local use="docker buildx use ${LANUZA_BUILDX_DRIVER}"
  debug "${use}"
  if ! command ${use} &> /dev/null ; then
    local create="docker buildx create --name ${LANUZA_BUILDX_DRIVER} --use --bootstrap --driver-opt=image=${LANUZA_BUILDX_DRIVER_IMAGE}"
    debug "${create}"
    command ${create} &> /dev/null
  fi
  # Booting builder before execution to avoid failures when building multiple services in docker compose.
  local inspect="docker buildx inspect --bootstrap"
  debug "${inspect}"
  command ${inspect} &> /dev/null
}

# Checks for failed containers and writes logs for them the the provided dir
function _docker_compose_save_logs() {
  local logdir="${LANUZA_OUTPUT_DIR}/docker-compose-logs"

  [[ -d "$logdir" ]] && rm -rf "$logdir"
  mkdir -p "$logdir"

  echoerr "Saving logs to ${logdir}..."

  # Get list of container if to service labels
  containers=()
  while IFS=$'\n' read -r container ; do
    [[ -n "$container" ]] && containers+=("$container")
  done < <(_docker_ps_by_project --format '{{.ID}}\t{{.Names}}')

  # Iterate over containers, handling empty container array as a possibility
  for line in "${containers[@]+"${containers[@]}"}"; do
    # Split tab-delimited tokens into service name and container id
    container_id="$(cut -d$'\t' -f1 <<< "$line")"
    container_name="$(cut -d$'\t' -f2 <<< "$line")"

    docker logs --timestamps "$container_id" &> "${logdir}/${container_name}.log"
  done
}
