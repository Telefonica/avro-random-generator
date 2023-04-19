#!/usr/bin/env bash

ARG_DEFS=(
  "[--build=(true|false)]"
  "[--version=(.+)]"
)

function init() {
  export BUILD="${BUILD:-"true"}"
  export VERSION="${VERSION:-"1.0.0"}"
  export REGISTRY="baikalpublicimages.azurecr.io"
  export SERVICE="avro-random-generator"
  export COMPOSE_FILE="./docker-compose.yml"
}

function run() {
  if [[ "${BUILD}" == "true" ]]; then
    ./lanuza/pipelines/build.sh --version="${VERSION}"
  fi

  docker-compose push app
}

source $(dirname $0)/../base.inc
