#!/bin/bash
source "${BASH_SOURCE%/*}/_path.sh"

set -e

docker-compose -f ${path_docker_compose_yml} build go-lint
docker-compose -f ${path_docker_compose_yml} run --rm go-lint
