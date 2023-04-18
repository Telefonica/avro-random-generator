#!/usr/bin/env bash

# Number of retries to check if an artifact exists
LANUZA_ARTIFACT_RETRIES=${LANUZA_ARTIFACT_RETRIES:-2}
# Initial backoff timeout in seconds to check if an artifact exists when the
# command has failed. Successive backoffs double the timeout.
LANUZA_ARTIFACT_RETRY_TIMEOUT=${LANUZA_ARTIFACT_RETRY_TIMEOUT:-1}

# Checks if an artifact type exists
# echoes
# - "published" when the artifact is published
# - "missing" when the artifact is missing
# - "unknown" when the artifact type is not handled
# - "failed" then the artifact checking fails (and fails with exit code 1)
function artifact_exists() {
  local type="${1}"
  local name="${2}"

  # Support for defining artifact checks in userland
  # Build the artifact checker dinamically and use that function, so
  # the installation repo can have its own checkers defined in ./usr
  # and do not force to have a lanuza release to support it
  local fn="artifact_exists_${type}"

  if [[ $(type -t "${fn}") == function ]]; then
    echoerr "Checking \"${type}\" artifact: \"${name}\""
    if ! ${fn} "${name}"; then
      echo "failed"
      fail "Checking \"${type}\" artifact failed: \"${name}\""
    fi
  else
    debug "There is not any function handler for checking \"${type}\" artifacts type "
    echo "unknown"
  fi
}

# Checks if a docker image exists in the registry
function artifact_exists_docker() {
  local image="${1}"

  function exists() {
    docker manifest inspect "${image}"
  }

  # Docker error on missing artifact is exit code == 1 and stderr:
  # no such manifest: ${image}
  _artifact_check_with_retries exists "no such manifest"
}

# Checks if a file exists in the s3
function artifact_exists_s3() {
  local url="${1}"
  local protocol="s3://"
  # Remove the protocol
  local url_no_protocol=$(echo "${url/${protocol}/}")
  # Extract the bucket
  local bucket=$(echo "${url_no_protocol}" | cut -d"/" -f1)
  # get the key (removing the trailing /)
  local key=$(echo "${url/${protocol}${bucket}\//}")

  function exists() {
    aws.sh s3api head-object --bucket "${bucket}" --key "${key}"
  }

  # aws error on missing file is exit code == 1 and stderr:
  # An error occurred (404) when calling the HeadObject operation: Not Found
  _artifact_check_with_retries exists "Not Found"
}

# Checks if a helm chart exists in the registry
# uses the docker cli as helm registries are OCI registry just like docker
function artifact_exists_helm() {
  local image="${1}"

  function exists() {
    docker manifest inspect "${image}"
  }

  # Docker error on missing artifact is exit code == 1 and stderr:
  # no such manifest: ${image}
  _artifact_check_with_retries exists "no such manifest"
}

# Retries an exists command a with backoff.
#
# The retry count is given by LANUZA_ARTIFACT_RETRIES (default 5), the
# initial backoff timeout is given by LANUZA_ARTIFACT_RETRY_TIMEOUT in seconds
# (default 1.)
#
# Successive backoffs double the timeout.
#
function _artifact_check_with_retries() {
  local fn=${1}
  local test_str=${2}
  local timeout=${LANUZA_ARTIFACT_RETRY_TIMEOUT}
  local error
  local attempt=0
  local exitCode=0

  while true; do
    if error=$($fn 2>&1); then
      exitCode=0
      echo "published"
      break
    elif [[ ${error} =~ "${test_str}" ]]; then
      exitCode=0
      echo "missing"
      break
    else
      exitCode=1
      attempt=$(( attempt + 1 ))
      debug "Command failed: ${error}"
      # -lt: is less than
      if [[ ${attempt} -le ${LANUZA_ARTIFACT_RETRIES} ]]; then
        debug "Retry #${attempt} in ${timeout} seconds"
        sleep ${timeout}
        timeout=$(( timeout * 2 ))
      else
        break
      fi
    fi
  done

  return ${exitCode}
}
