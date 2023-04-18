#!/usr/bin/env -S -u TAGS bash

# list all workspace names
#
# Uses the shebang line to unset env vars that may cause interference before base.inc sourcing
# https://www.gnu.org/software/coreutils/manual/html_node/env-invocation.html#env-invocation

ARG_DEFS=(
  # comma separated list of tags to filter workspaces (OR)
  "[--tags=(.+)]"
)

function run() {
  workspace_list "${TAGS:-""}"
}

source $(dirname $0)/../base.inc
