#!/usr/bin/env bash

function init() {
  export VERSION="${VERSION:-snapshot}"
  export SERVICE="avro-random-generator"
  export COMPOSE_FILE="./docker-compose.yml"
}

function run() {
  docker-compose build app 1>&2
  docker-compose run app "$@"
}

source $(dirname $0)/../base.inc
