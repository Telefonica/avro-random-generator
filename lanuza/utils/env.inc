#!/usr/bin/env bash

# exports all environment variables in an .env file
# If no parameters are provided, it will use the current directory .env file 
# usage:
# env_load 
# env_load ./config/profiles/file.env
function env_load() {
  local file=${1:-.env}

  [ ! -f "${file}" ] && echo "${file} file not found" && return 1
  # Load profile
  # https://stackoverflow.com/questions/19331497/set-environment-variables-from-file-of-key-value-pairs
  set -a
  # https://unix.stackexchange.com/questions/176873/feed-source-command-with-a-pipe
  source "${file}"
  set +a
}
