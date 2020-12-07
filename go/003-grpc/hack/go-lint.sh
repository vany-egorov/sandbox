#!/bin/bash
source "${BASH_SOURCE%/*}/_path.sh"
source "${path_script}/_compile-proto.sh"
source "${path_script}/_mock-generate.sh"

compile_proto
mock_generate

golangci-lint run \
  -v \
  --max-same-issues 1000 \
  --max-issues-per-linter 1000 \
  ./cmd/... \
  ./internal/... \
  ./appversion/... ;

exit 0
