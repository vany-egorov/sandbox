#!/bin/bash
source "${BASH_SOURCE%/*}/_path.sh"

export GOROOT="${path_env}/go"
export GOPATH="${path_env}/gopath"
export PATH="${GOROOT}/bin:${GOPATH}/bin:${PATH}"
