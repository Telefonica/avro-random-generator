#!/usr/bin/env bash

export COMPOSE_DOCKER_CLI_BUILD="${COMPOSE_DOCKER_CLI_BUILD:-1}"
export COMPOSE_PATH_SEPARATOR="${COMPOSE_PATH_SEPARATOR:-:}"
export COMPOSE_HTTP_TIMEOUT="${COMPOSE_HTTP_TIMEOUT:-300}"
# reset compose to allow pipelines nested calling without host interferences
export COMPOSE_PROJECT_NAME="${LANUZA_PROJECT}_${LANUZA_BUILD_ID}"
# Setting to ="" causes "docker compose" (v2) to fail
unset COMPOSE_PROFILES
unset COMPOSE_FILE

# flag to know when should we autocleanup docker-compose
LANUZA_DOCKER_COMPOSE_LEGACY__CLEANUP=0
# array of internally created temp_files that should be removed on cleanup
temp_files=()

# Decorate docker-compose command
function docker-compose() {
  LANUZA_DOCKER_COMPOSE_LEGACY__CLEANUP=1

  # docker-compose run ...
  if [ $1 == "run" ]; then
    _docker-compose_run "$@"

  # docker-compose build ...
  elif [ $1 == "build" ]; then
    _docker-compose_build "$@"

  else
    _docker-compose "$@"
  fi
}

# tags a built docker-compose service
# @deprecated: docker compose v2 has builtin support for tags in the docker-compose.yml file
# Usage:
# # When the docker-compose service has an "image" property
# - local dest_name=$(docker_compose_tag "service" "latest")
#   dest_name=="image_value:latest"
# - local dest_name=$(docker_compose_tag "service" "latest" "example.com/repo")
#   dest_name=="example.com/repo/service:latest"
function docker_compose_tag() {
  local service=$1
  local tag=$(_docker-compose_normalize_tag "$2")
  local dest_image=${3:-$(_docker-compose_image_for_service $service)}

  # remove the tag part (:latest on "alpine:latest")
  local dest_name=$(echo $dest_image | sed -e "s/:.*$//")

  dest_name="${dest_name}:${tag}"
  # try to get how compose named the image to retag it
  # when an "image" property is present, it will use it as name
  local local_name=$(_docker-compose_image_for_service "$service")
  if [[ "$local_name" == "" ]]; then
    # when no "image" is present, it will use this consensus
    local_name="${COMPOSE_PROJECT_NAME}_${service}"
  fi

  debug "tagging ${local_name} --> ${dest_name}"
  command docker tag "${local_name}" "${dest_name}"
  echo "${dest_name}"
}

function _docker-compose_cleanup() {
  if [ "$LANUZA_DOCKER_COMPOSE_LEGACY__CLEANUP" -eq 1 ]; then
    _docker-compose down -v --rmi local --remove-orphans || true
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
function _docker-compose_normalize_tag() {
  local tag=$1
  echo "${tag}" | sed 's/[^a-zA-Z0-9_\-\.]/-/g' | cut -c1-128
}

# Gets a the image (if any) for a docker-compose service.
# local image_name=$(_docker-compose_image_for_service "$service")
function _docker-compose_image_for_service() {
  local service="$1"
  local prop=""

  prop=$(docker-compose config \
    | grep -E "^(  [._[:alnum:]-]+:|    image:)" \
    | grep -E "(  ${service}:)" -A 1 \
    | grep -oE "  image: (.+)" \
    | awk '{print $2}')

  echo "$prop"
}

# Checks if the property exists in a docker-compose service
# Usage:
# if _docker-compose_property_exists_for_service "build" "service"; then
#   echo "exists"
# else
#   echo "does not exists"
# fi
function _docker-compose_property_exists_for_service() {
  local property="$1"
  local service="$2"

  local exists=$(docker-compose config \
    | grep -E "^(  [._[:alnum:]-]+:|    ${property}:)" \
    | grep -E "(  ${service}:)" -A 1 \
    | grep -oE "  ${property}:")

  if [[ "${exists}" == "" ]]; then
    return 1
  else
    return 0
  fi
}

# executes docker-compose command with defaults
function _docker-compose() {
  debug "docker-compose $@"
  command docker-compose "$@"
}

# docker-compose build handler
function _docker-compose_build() {
  function _build() {
    # set standard labels https://github.com/opencontainers/image-spec/blob/main/annotations.md
    local yaml=$'\n'"version: '3.6'"$'\n'"services:"$'\n'
    local date="$(date -u +"%Y-%m-%dT%H:%M:%SZ")"
    local version="$(git_get_version)"
    local revision="$(git_get_sha)"

    for service in ${services[@]}; do
      yaml+="  ${service}:"$'\n'
      yaml+="    build:"$'\n'
      yaml+="      labels:"$'\n'
      yaml+="        org.opencontainers.image.created: \"$date\""$'\n'
      yaml+="        org.opencontainers.image.version: \"${version}"\"$'\n'
      yaml+="        org.opencontainers.image.revision: \"${revision}\""$'\n'
    done

    debug "Using the following docker-compose file to specify labels"
    debug "$yaml"
    # Include new yaml in the docker-compose files
    temp_compose=$(mktemp "${TMPDIR:-/tmp/}"docker-compose.labels.$LANUZA_BUILD_ID.XXXXXXXXX)
    # To cleanup at exit
    temp_files+=( $temp_compose )
    echo "$yaml" > $temp_compose
    COMPOSE_FILE=${COMPOSE_FILE:-docker-compose.yml}
    export COMPOSE_FILE="${COMPOSE_FILE}${COMPOSE_PATH_SEPARATOR}${temp_compose}"

    # build automatically with VERSION
    set -- "build" "--build-arg" "VERSION=${VERSION:-snapshot}" "${@:2}"
    set -- "build" "--build-arg" "DOCKER_UID=${DOCKER_UID}" "${@:2}"
    set -- "build" "--build-arg" "DOCKER_GID=${DOCKER_GID}" "${@:2}"

    # compatibility mode: Use the default buildx builder (docker native)
    _docker buildx use default || true
    _docker-compose "$@"
  }

  # get services affected by build
  local services=()
  if [ -z ${2+x} ]; then
    # use all docker-compose services stated in the compose files
    services+=( $(docker-compose config --services) )
  else
    # we have an specific "docker-compose build <service>". Use that service in our cache
    services+=($2)
  fi

  # remove all services that are not buildable
  for index in ${!services[@]}; do
    local service=${services[$index]}
    if ! _docker-compose_property_exists_for_service "build" "${service}"; then
      debug "${service} uses an image, skipping"
      unset services[$index]
    fi
  done

  if [[ -z ${LANUZA_INLINE_CACHE_REPO+x} ]]; then
    # no autocache feature when has not been configured in the repository
    _build "$@"
  else

    local dest_image_prefix
    if workspace_enabled; then
      dest_image_prefix="${LANUZA_INLINE_CACHE_REPO}/$(workspace_name)"
    else
      dest_image_prefix="${LANUZA_INLINE_CACHE_REPO}/${LANUZA_PROJECT}"
    fi
    # activate build inline cache when LANUZA_INLINE_CACHE_REPO is set
    debug "using the auto inline cache feature for building"

    # store our target branches in array
    IFS=':' read -r -a branches <<< "${LANUZA_INLINE_CACHE_BRANCHES:-master}"

    # generate the yaml with cache-from for the branches and services
    local yaml=$'\n'"version: '3.6'"$'\n'"services:"$'\n'
    for service in ${services[@]}; do
      local dest_image="${dest_image_prefix}_${service}"
      local caches=""
      for branch in ${branches[@]}; do
        local tag=$(_docker-compose_normalize_tag "$branch")
        caches+="        - ${dest_image}:${tag}"$'\n'
      done
      yaml+="  ${service}:"$'\n'
      yaml+="    build:"$'\n'
      yaml+="      cache_from:"$'\n'
      yaml+="${caches}"
    done

    debug "Using the following docker-compose file to specify caches"
    debug "$yaml"

    # Include new yaml in the docker-compose files
    temp_compose=$(mktemp "${TMPDIR:-/tmp/}"docker-compose.cache.$LANUZA_BUILD_ID.XXXXXXXXX)
    # To cleanup at exit
    temp_files+=( $temp_compose )
    echo "$yaml" > $temp_compose
    COMPOSE_FILE=${COMPOSE_FILE:-docker-compose.yml}
    export COMPOSE_FILE="${COMPOSE_FILE}${COMPOSE_PATH_SEPARATOR}${temp_compose}"

    # build and push to cache the service only when we are executing in CI and in a branch with cache enabled
    local branch=$(git_get_branch)
    if [[ ! -z ${CI+x} ]] && [[ " ${branches[*]} " =~ " ${branch} " ]]; then
      # we are on a branch that should be cached
      debug "inline docker cache push for current branch \"${branch}\""

      # activate inline docker cache
      set -- "build" "--build-arg" "BUILDKIT_INLINE_CACHE=1" "${@:2}"

      _build "$@"

      # push the image to cache
      for service in ${services[@]}; do
        # retag service
        local dest_image="${dest_image_prefix}_${service}"
        local dest_name=$(docker_compose_tag $service $branch $dest_image)
        debug "pushing $dest_name"
        command docker push $dest_name || true
      done

    else
      _build "$@"
    fi
  fi
}

# docker-compose run handler
function _docker-compose_run() {
  LANUZA_SAVE_CONTAINER_LOGS="${LANUZA_SAVE_CONTAINER_LOGS:-"on-error"}"

  # Allow the host user to run commands in docker
  # propagate internal env variables to the run commands
  set -- "run" "-e" "DOCKER_UID=$DOCKER_UID" "${@:2}"
  set -- "run" "-e" "DOCKER_GID=$DOCKER_GID" "${@:2}"
  set -- "run" "-e" "LANUZA_BUILD_ID=$LANUZA_BUILD_ID" "${@:2}"
  set -- "run" "-e" "DEBUG" "${@:2}"

  set +e
  _docker-compose "$@"
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
    _docker-compose_save_logs
  fi

  if [[ "${LANUZA_SAVE_CONTAINER_LOGS}" =~ ^(on-error)$ ]] && [[ 0 != "${#failed_containers[@]}" ]]; then
    _docker-compose_save_logs
  fi

  if [[ $exitcode -ne 0 ]] ; then
    exit $exitcode
  fi
}

# Checks for failed containers and writes logs for them the the provided dir
function _docker-compose_save_logs() {
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
