#!/bin/bash
#
# provides target enum
source "${BASH_SOURCE%/*}/_path.sh"


TARGET_NODE=0

target=$TARGET_NODE


target_parse() {
  local t=$TARGET_NODE

  case "$1" in
    node) t=$TARGET_NODE; ;;
    *) ;;
  esac

  echo $t
}

target_name() {
  local n=""

  case $1 in
    $TARGET_NODE) n="node"; ;;
    *) ;;
  esac

  echo $n
}

# output binary name
target_go_out() {
  target_name $1
}

# alias to target_go_out
target_binary_name() {
  target_go_out $1
}


# full path to binary ".../bin/..."
target_go_out_fullpath() {
  echo "${path_bin}/$(target_go_out $1)"
}

# path to all main package go files
target_go_main_files() {
  echo "${path_cmd}/$(target_go_out $1)/main.go"
}

target_docker_target() {
  local dt="build"
  echo $dt
}

#    docker tag name
# or docker container name
target_docker_name() {
  local dn=$(target_docker_target $1)
  echo "raft-memberlist-${dn}"
}
