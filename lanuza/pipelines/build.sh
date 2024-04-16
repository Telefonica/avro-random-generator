#!/usr/bin/env bash

ARG_DEFS=(
  "[--version=(.+)]"
)

function init() {
  export VERSION="${VERSION:-snapshot}"
  export SERVICE="avro-random-generator"
  export COMPOSE_FILE="./docker-compose.yml"
}

function run() {
  docker compose build app
  docker compose run app ./lanuza/scripts/build.sh
}

source $(dirname $0)/../base.inc
