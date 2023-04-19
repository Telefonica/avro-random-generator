#!/usr/bin/env -S -u PIPELINE bash

# executes a pipeline in all workspaces
#
# Uses the shebang line to unset env vars that may cause interference before base.inc sourcing
# https://www.gnu.org/software/coreutils/manual/html_node/env-invocation.html#env-invocation

ARG_DEFS=(
  "--pipeline=(.+)"
)

function run() {
  workspace_run "$PIPELINE"
}

source $(dirname $0)/../base.inc
