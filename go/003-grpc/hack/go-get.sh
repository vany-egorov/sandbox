#!/bin/bash
source "${BASH_SOURCE%/*}/golangci-lint-version.sh"

go_get_libs() {
  # install libraries and utils
  go get -u -v "github.com/golang/protobuf/proto"
  go get -u -v "github.com/golang/protobuf/protoc-gen-go"
  go install   "google.golang.org/grpc/cmd/protoc-gen-go-grpc"
  go get -u -v "github.com/mfridman/tparse"
  go get -u -v "github.com/t-yuki/gocover-cobertura"
  go get       "github.com/golang/mock/mockgen@v1.4.4"
}

go_get_golangci_lint() {
  curl -sSfL https://raw.githubusercontent.com/golangci/golangci-lint/master/install.sh | \
    sh -s -- -b $(go env GOPATH)/bin "v${golangci_lint_version}"
}

go_get() {
  go_get_libs
  go_get_golangci_lint
}

main() {
  go_get
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
  main "$@"
fi
