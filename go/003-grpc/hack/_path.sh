#!/bin/bash
#
# provides various paths
path_script=$(readlink -f ${BASH_SOURCE%/*})
path_project=$(readlink -f "${path_script}/..")
path_coverage="${path_project}/coverage"
path_cover_out="${path_coverage}/cover.out"
path_cover_html="${path_coverage}/cover.html"
path_cobertura_coverage="${path_coverage}/cobertura-coverage.xml"
path_bin=$(readlink -f "${path_project}/bin")
path_cmd=$(readlink -f "${path_project}/cmd")
path_env=$(readlink -f "${path_project}/env")
path_hack=$(readlink -f "${path_project}/hack")
path_version=$(readlink -f "${path_project}/VERSION")
path_docker=$(readlink -f "${path_project}/docker")
path_docker_compose_yml=$(readlink -f "${path_project}/docker-compose.yml")
