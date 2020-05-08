#!/bin/bash
#
# provides various paths
path_script=$(readlink -f ${BASH_SOURCE%/*})
path_project=$(readlink -f "${path_script}/..")
path_bin=$(readlink -f "${path_project}/bin")
path_cmd=$(readlink -f "${path_project}/cmd")
path_env=$(readlink -f "${path_project}/env")
path_version=$(readlink -f "${path_project}/VERSION")
path_docker=$(readlink -f "${path_project}/docker")
