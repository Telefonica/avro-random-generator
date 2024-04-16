#!/usr/bin/env bash

#@IgnoreInspection BashAddShebang
#
# LANUZA v4.14.0
#
# This file provides:
# - a default control flow
#   * initializes the environment
#   * call a function in your script based on the arguments
# - named argument parsing and automatic generation of the "usage" for your script
# - library functions automatically available when included in './lib' directory
# - user functions automatically available when included in './usr' directory
#
# Usage:
# - define a "run" function
# - define the optional variable ARG_DEFS (see below) with the arguments for your script
# - define an optional "init" function
# - define and optional "cleanup" function
# - include this file using `source $(dirname $0)/../base.inc` at the end of your script.
#
# Default control flow:
# 1. Set the current directory to the root directory of the script. By this
#    the script can be called from anywhere.
# 2. Parse the named arguments
# 3. The function "init" will be called if it exists
# 4. The function "run" will be called
# 5. Any function ending with "cleanup" will be called if it exists when the execution ends with or without error
#
# Named Argument Parsing:
# - The variable ARG_DEFS defines the valid command arguments
#   * Required args syntax: --paramName=paramRegex
#   * Optional args syntax: [--paramName=paramRegex]
#   * e.g. ARG_DEFS=("--required_param=(.+)" "[--optional_param=(.+)]")
# - Checks that:
#   * all arguments match to an entry in ARG_DEFS
#   * all required arguments are present
#   * all arguments match their regex
# - Afterwards, every parameter value will be stored in a variable
#   with the name of the parameter in upper case (with dash converted to underscore).
#
# Initially taken from https://github.com/angular/angular.js/blob/593b18c66a43414b3caaa53f8a1e438e79d00e95/scripts/utils.inc
# MIT Licensed

# always stop on errors
set -e

function usage {
  echoerr "Usage: ${0} ${ARG_DEFS[@]}"
  exit 1
}

function echoerr {
  echo "$@" 1>&2;
}

function isDebugEnabled {
  if [[ "${DEBUG:-""}" == *"lanuza"* ]]; then
    return 0
  else
    return 1
  fi
}

# prints a debug statement if a DEBUG env var containing "lanuza" is present
function debug {
  if isDebugEnabled; then echoerr "[lanuza] $@"; fi
}

# prints a red message to stderr and exits
function fail {
  echo -e "\033[0;31m[lanuza] $@\033[0m" 1>&2;
  exit 1
}

# gets the lanuza root dir
# https://www.ostricher.com/2014/10/the-right-way-to-get-the-directory-of-a-bash-script/
function get_root_dir {
  local SOURCE DIR

  # absolute pathname of this script
  SOURCE="$( _realpath "${BASH_SOURCE[0]}" )"

  # print absolute pathname of parent directory
  DIR="$( cd -P "$( dirname "$SOURCE" )/../" && pwd )"
  echo "$DIR"
}

# gets the canonicalised absolute pathname of the argument, resolving symlinks
# realpath(1) is part of GNU Coreutils, not always available as preinstalled command in other Unix variants
function _realpath {
  local NAME="$1"
  local DIR

  # While $NAME is a symlink, resolve it
  while [[ -h "$NAME" ]]; do
    DIR="$( cd -P "$( dirname "$NAME" )" && pwd )"
    NAME="$( readlink "$NAME" )"
    # If $NAME was a relative symlink (so no "/" as prefix), need to resolve it relative to the symlink base directory
    [[ $NAME != /* ]] && NAME="$DIR/$NAME"
  done

  # Resolve relative paths in $NAME and print final absolute pathname
  [[ -d "$NAME" ]] && NAME="$( cd -P "$NAME" && pwd )"
  DIR="$( cd -P "$( dirname "$NAME" )" && pwd )"
  echo "${DIR%/}/$( basename "${NAME%/}" )"
}

function parseArgs {
  local REQUIRED_ARG_NAMES=()

  # -- helper functions
  function varName {
    # everything to upper case and dash to underscore
    echo ${1//-/_} | tr '[:lower:]' '[:upper:]'
  }

  function readArgDefs {
    local ARG_DEF
    local AD_OPTIONAL
    local AD_NAME
    local AD_RE

    # -- helper functions
    function parseArgDef {
      local ARG_DEF_REGEX="(\[?)--([^=]+)=(.*)"
      if [[ ! $1 =~ $ARG_DEF_REGEX ]]; then
        echoerr "Internal error: arg def has wrong format: $ARG_DEF"
        exit 1
      fi
      AD_OPTIONAL="${BASH_REMATCH[1]}"
      AD_NAME="${BASH_REMATCH[2]}"
      AD_RE="${BASH_REMATCH[3]}"
      if [[ $AD_OPTIONAL ]]; then
        # Remove last bracket for optional args.
        # Can't put this into the ARG_DEF_REGEX somehow...
        AD_RE=${AD_RE%?}
      fi
    }

    # -- run
    for ARG_DEF in "${ARG_DEFS[@]}"; do
      parseArgDef $ARG_DEF

      local AD_NAME_UPPER=$(varName $AD_NAME)
      setVar "${AD_NAME_UPPER}_OPTIONAL" "$AD_OPTIONAL"
      setVar "${AD_NAME_UPPER}_RE" "$AD_RE"
      if [[ ! $AD_OPTIONAL ]]; then
        REQUIRED_ARG_NAMES+=($AD_NAME)
      fi
    done
  }

  function readAndValidateArgs {
    local ARG_NAME
    local ARG_VALUE
    local ARG_NAME_UPPER

    # -- helper functions
    function parseArg {
      local ARG_REGEX="--([^=]+)=?(.*)"

      if [[ ! $1 =~ $ARG_REGEX ]]; then
        echoerr "Can't parse argument $i"
        usage
      fi

      ARG_NAME="${BASH_REMATCH[1]}"
      ARG_VALUE="${BASH_REMATCH[2]}"
      ARG_NAME_UPPER=$(varName $ARG_NAME)
    }

    function validateArg {
      local AD_RE=$(getVar ${ARG_NAME_UPPER}_RE)

      if [[ ! $AD_RE ]]; then
        echoerr "Unknown option: $ARG_NAME"
        usage
      fi

      if [[ ! $ARG_VALUE =~ ^${AD_RE}$ ]]; then
        echoerr "Wrong format: $ARG_NAME"
        usage;
      fi
    }

    # -- run
    for i in "$@"; do
      parseArg "$i"
      validateArg
      setVar "${ARG_NAME_UPPER}" "$ARG_VALUE"
    done
  }

  function checkMissingArgs {
    local ARG_NAME
    for ARG_NAME in "${REQUIRED_ARG_NAMES[@]}"; do
      ARG_VALUE=$(getVar $(varName $ARG_NAME))

      if [[ ! $ARG_VALUE ]]; then
        echoerr "Missing: $ARG_NAME"
        usage;
      fi
    done
  }

  # -- run
  # Only parse args if there are some of them defined
  if (( ${#ARG_DEFS[@]} )); then
    readArgDefs
    readAndValidateArgs "$@"
    checkMissingArgs
  fi
}

# getVar(varName)
function getVar {
  echo ${!1}
}

# setVar(varName, varValue)
function setVar {
  eval "export $1=\"$2\""
}

# isFunction(name)
# - to be used in an if, so return 0 if successful and 1 if not!
function isFunction {
  if [[ $(type -t $1) == "function" ]]; then
    return 0
  else
    return 1
  fi
}

# Generate a pseudo UUID
function gen_lanuza_id() {
  # Using /dev/urandom hangs the process on GitHub Actions, and we do not need such great entropy
  # based on https://gist.github.com/markusfisch/6110640
  local N B C='89ab'

  for (( N=0; N < 6; ++N )); do
    B=$(( $RANDOM%256 ))

    case $N in
      1)
        printf '4%x' $(( B%16 ))
        ;;
      3 | 5)
        printf '%c%x' ${C:$RANDOM%${#C}:1} $(( B%16 ))
        ;;
      *)
        printf '%02x' $B
        ;;
    esac
  done

  echo
}

function main {
  function cleanup__() {
    local exit_code=$?

    mapfile -t cleanup_functions < <( declare -F | grep -E 'cleanup$' | tr -s ' ' | cut -d ' ' -f3 )

    for cleanup in "${cleanup_functions[@]}"; do
      $cleanup $exit_code|| true
    done
  }

  set -Eeo pipefail
  set +x

  setVar "LANUZA_ROOT" "$(get_root_dir)"

  # caller 1 # contains the caller script
  local source=$(_realpath $(caller 1 | tr -s ' ' | cut -d ' ' -f3 ))
  local rootless_source="${source#"${LANUZA_ROOT}/"}"
  setVar "LANUZA_SOURCE" "$rootless_source"
  setVar "LANUZA_SOURCE_FILE" "$(basename $source .sh)"

  # a random number tied to this execution
  setVar "LANUZA_BUILD_ID" "${LANUZA_BUILD_ID:-$(gen_lanuza_id)}"
  setVar "LANUZA_UUID" "$(gen_lanuza_id)"

  # Output directory to save lanuza generated files
  setVar "LANUZA_OUTPUT_DIR" "${LANUZA_OUTPUT_DIR:-"output"}"

  # change directory to base directory of the project
  BASE_DIR=$(dirname $BASH_SOURCE)
  cd $BASE_DIR/..;

  local current_dir=$(_realpath $PWD)
  local project_name
  if [[ ${LANUZA_ROOT} == ${current_dir} ]]; then
    # use the root dir as project name
    project_name=$(basename "${LANUZA_ROOT}")
  else
    # The relative path from root to project without /
    project_name=$(echo ${current_dir#"${LANUZA_ROOT}/"} | tr '/' _ )
  fi

  setVar "LANUZA_PROJECT" "${project_name}"

  local f
  # load other utilities
  for f in $(find ${LANUZA_ROOT}/lanuza/utils -name '*.inc'); do
    source "$f";
  done

  # load ci overrides
  for f in $(find ${LANUZA_ROOT}/lanuza/utils/ci -name '*.inc'); do
    source "$f";
  done

  # load usr libs
  for f in $(find ${LANUZA_ROOT}/lanuza/usr -name '*.inc'); do
    source "$f";
  done

  debug "=== $LANUZA_SOURCE ==="

  parseArgs "$@"

  set -u

  trap cleanup__ 0 2 3 15

  if isFunction init; then
    init "$@"
  fi

  # call the "run" function
  run "$@"
}

main "$@"
