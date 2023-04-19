#!/usr/bin/env -S -u WORKSPACE -u PIPELINE bash

# executes a pipeline in a workspace
#
# Uses the shebang line to unset env vars that may cause interference before base.inc sourcing
# https://www.gnu.org/software/coreutils/manual/html_node/env-invocation.html#env-invocation

ARG_DEFS=(
  "--pipeline=(.+)"
  "--workspace=(.+)"
)

function run() {
  workspace_exec "$WORKSPACE" "$PIPELINE"
}

source $(dirname $0)/../base.inc
