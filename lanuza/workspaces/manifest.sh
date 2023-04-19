#!/usr/bin/env -S -u WORKSPACE bash

# returns the workspace manifest
#
# Uses the shebang line to unset env vars that may cause interference before base.inc sourcing
# https://www.gnu.org/software/coreutils/manual/html_node/env-invocation.html#env-invocation

ARG_DEFS=(
  "--workspace=(.+)"
)

function run() {
  workspace_manifest "$WORKSPACE"
}

source $(dirname $0)/../base.inc
