#!/bin/bash
source "${BASH_SOURCE%/*}/_path.sh"

set -e

docker-compose -f ${path_docker_compose_yml} rm -f go-test

mkdir -vp "${path_project}/coverage"

docker-compose -f ${path_docker_compose_yml} build go-test
docker-compose -f ${path_docker_compose_yml} run --name grpcexample-go-test go-test

docker cp "grpcexample-go-test:/usr/local/grpcexample/coverage/." "${path_coverage}"

docker-compose -f ${path_docker_compose_yml} rm -f go-test
