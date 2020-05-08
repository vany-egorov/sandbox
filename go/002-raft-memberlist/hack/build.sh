#!/bin/bash
source "${BASH_SOURCE%/*}/_path.sh"
source "${path_script}/_target.sh"


main() {
  target=$(target_parse $1)

  go_out=$(target_go_out_fullpath $target)
  go_main_files=$(target_go_main_files $target)

  xpkg="github.com/vany-egorov/raftmemberlistexample/appversion"
  version="$(head -1 $path_version)"
  go_build_date=$(date -u +%Y-%m-%dT%H:%M:%S)
  go_os="linux"
  go_arch="amd64"

  # ensure bin dir
  [ -d "${path_bin}" ] || mkdir -pv "${path_bin}"

  go_ldflags="-s -w -X \"${xpkg}.BuildDate=${go_build_date}\" -X \"${xpkg}.Version=${version}\" -extldflags -static"

  GOOS=${GOOS} GOARCH=${GOARCH} go build \
    -v \
    -ldflags "${go_ldflags}" \
    -o $go_out \
      $go_main_files
}

set -e

main "$@"
