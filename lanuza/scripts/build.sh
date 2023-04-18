#!/usr/bin/env bash

function run() {
  ./gradlew build
}

source $(dirname $0)/../base.inc
