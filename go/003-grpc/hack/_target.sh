#!/bin/bash
#
# provides target enum
source "${BASH_SOURCE%/*}/_path.sh"


TARGET_BROKER=0
TARGET_WORKER=1

target=$TARGET_BROKER


target_parse() {
  local t=$TARGET_BROKER

  case "$1" in
    broker) t=$TARGET_BROKER; ;;
    worker) t=$TARGET_WORKER; ;;
    *) ;;
  esac

  echo $t
}

target_name() {
  local n=""

  case $1 in
    $TARGET_BROKER) n="broker"; ;;
    $TARGET_WORKER) n="worker"; ;;
    *) ;;
  esac

  echo $n
}

# output binary name
target_go_out() {
  local go="broker"

  case $1 in
    $TARGET_BROKER) go="broker"; ;;
    $TARGET_WORKER) go="worker"; ;;
  esac

  echo $go
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
  local dt="build-$(target_name $1)"

  echo $dt
}

#    docker tag name
# or docker container name
target_docker_name() {
  local dn=$(target_docker_target $1)
  echo "grpcexample-${dn}"
}
