#!/bin/bash
source "${BASH_SOURCE%/*}/_path.sh"
source "${BASH_SOURCE%/*}/go-version.sh"


path=${path_env}


usage()  {
  cat <<EOF
Usage: $(basename "$0") [name] [options...]
Options:
  -h, --help | print help message;
  -p, --path | install golang to path;
EOF

1>&2;
}


parse_arguments() {
  local options=hp:
  local longopts=help,path:

  # -use ! and PIPESTATUS to get exit code with errexit set
  # -temporarily store output to be able to check for errors
  # -activate quoting/enhanced mode (e.g. by writing out “--options”)
  # -pass arguments only via   -- "$@"   to separate them correctly
  ! PARSED=$(getopt --options=$options --longoptions=$longopts --name "$0" -- "$@")
  if [[ ${PIPESTATUS[0]} -ne 0 ]]; then
    # e.g. return value is 1
    #  then getopt has complained about wrong arguments to stdout
    exit 2
  fi
  # read getopt’s output this way to handle the quoting right:
  eval set -- "$PARSED"

  while true; do
    case "$1" in
      -p | --path) path="$2"; shift 2 ;;
      -h | --help) usage; exit 1 ;;
      --) break; ;;
      *) usage; echo ""; echo "unknow option/argument $1; see --help" 1>&2; exit 1; ;;
    esac
  done
}


main() {
  parse_arguments "$@"

  echo "==> will install golang to ${path}"

  # ensure env dir
  [ -d "${path}" ] || mkdir -pv "${path}"
  [ -d "${path}/gopath" ] || mkdir -pv "${path}/gopath"

  # install golang
  if [ ! -d "${path}/go" ]; then
    cd "${path}"

    curl -OL "https://dl.google.com/go/go${go_version}.linux-amd64.tar.gz" && \
    mv ./go*tar.gz ./go.tar.gz && \
    tar -xf ./go.tar.gz && \
    rm -rf ./go.tar.gz
  fi

  # install libraries
  go get -u -v "github.com/golang/protobuf/proto"
  go get -u -v "github.com/golang/protobuf/protoc-gen-go"
  go get -u -v "github.com/bronze1man/yaml2json"
  go get -u -v "github.com/jteeuwen/go-bindata"
  go get -u -v "github.com/jteeuwen/go-bindata/go-bindata"
}

set -e

main "$@"
