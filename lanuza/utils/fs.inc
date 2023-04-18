#!/usr/bin/env bash

# list the files from the current $PWD.
# All extra paramaters are passed to the underliying command rg
function fs_list() {
  rg.sh --files --follow "$@"
}
