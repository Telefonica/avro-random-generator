#!/usr/bin/env -S -u TAGS -u FORMAT bash

# list all workspace names
#
# Uses the shebang line to unset env vars that may cause interference before base.inc sourcing
# https://www.gnu.org/software/coreutils/manual/html_node/env-invocation.html#env-invocation

ARG_DEFS=(
  # comma separated list of tags to filter workspaces (OR)
  "[--tags=(.+)]"
  "[--format=(text|json)]"
)

function init() {
  TAGS="${TAGS:-""}"
  FORMAT="${FORMAT:-text}"
}

function run() {
  if [[ "${FORMAT}" == "json" ]]; then
    workspace_list "${TAGS}" | jq -cnR '[inputs | select(length>0)]'
  else
    workspace_list "${TAGS}"
  fi
}

source $(dirname $0)/../base.inc
