#!/bin/bash
#
# provides various paths
path_script=$(readlink -f ${BASH_SOURCE%/*})
path_project=$(readlink -f "${path_script}/..")
path_bin=$(readlink -f "${path_project}/bin")
path_env=$(readlink -f "${path_project}/env")
