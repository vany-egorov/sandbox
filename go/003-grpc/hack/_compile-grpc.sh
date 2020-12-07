#!/bin/bash
source "${BASH_SOURCE%/*}/_path.sh"


compile_grpc() {
  (
    cd ${path_project}/internal/pkg/service/ &&
    protoc \
      --go_out=. --go_opt=paths=source_relative \
      --go-grpc_out=. --go-grpc_opt=paths=source_relative \
      queue.proto
  )
}
