#!/usr/bin/env -S -u WORKSPACE -u MODE bash

# show the shasum for a workspace
#
# Uses the shebang line to unset env vars that may cause interference before base.inc sourcing
# https://www.gnu.org/software/coreutils/manual/html_node/env-invocation.html#env-invocation

ARG_DEFS=(
  "--workspace=(.+)"
  "[--mode=(standard|pr|canonical|component)]"
)

function init() {
  export MODE=${MODE:-"standard"}
}

function run() {
  workspace_get_sha "${WORKSPACE}" "${MODE}"
}

source $(dirname $0)/../base.inc
