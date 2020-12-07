#!/bin/bash
source "${BASH_SOURCE%/*}/_path.sh"
source "${path_script}/_target.sh"

copy_local=false
docker_buildkit=false

usage()  {
  cat <<EOF
Usage: $(basename "$0") [name] [options...]
Options:
  -h, --help                        | print help message;
  -k, --docker-buildkit, --buildkit | build via Docker Buildkit;
  -c, --copy, --copy-local          | copy build result to local bin directory;
EOF

1>&2;
}


parse_arguments() {
  local options=hck
  local longopts=help,copy,copy-local,docker-buildkit,buildkit

  # -use ! and PIPESTATUS to get exit code with errexit set
  # -temporarily store output to be able to check for errors
  # -activate quoting/enhanced mode (e.g. by writing out “--options”)
  # -pass arguments only via   -- "$@"   to separate them correctly
  ! PARSED=$(getopt --options=$options --longoptions=$longopts --name "$0" -- "$@")
  if [[ ${PIPESTATUS[0]} -ne 0 ]]; then
    # e.g. return value is 1
    #  then getopt has complained about wrong arguments to stdout
    exit 2
  fi
  # read getopt’s output this way to handle the quoting right:
  eval set -- "$PARSED"

  while true; do
    case "$1" in
      -c | --copy | --copy-local) copy_local=true; shift ;;
      -k | --buildkit | --docker-buildkit) docker_buildkit=true; shift ;;
      -h | --help) usage; exit 1 ;;
      --) break; ;;
      *) usage; echo ""; echo "unknow option/argument $1; see --help" 1>&2; exit 1; ;;
    esac
  done
}


main() {
  target=$(target_parse $1)
  parse_arguments "$@"

  name=$(target_binary_name $target)
  docker_name=$(target_docker_name $target)
  docker_target=$(target_docker_target $target)

  path_binary="${path_bin}/${name}"

  # use DOCKER_BUILDKIT
  if [ "$docker_buildkit" = true ]; then
    DOCKER_BUILDKIT=1 docker build \
      -f "${path_docker}/build/Dockerfile" \
      --target "${docker_target}" \
      --tag "${docker_name}" \
      .

    if [ "$copy_local" = true ]; then
      # ensure bin dir
      [ -d "${path_bin}" ] || mkdir -pv "${path_bin}"

      docker run --rm --entrypoint cat "${docker_name}" "/usr/local/bin/${name}" > "${path_binary}"

      chmod -v +x "${path_binary}"
    fi
  # use docker-compose
  else
    docker-compose -f "${path_docker_compose_yml}" up --remove-orphans --build "${docker_target}"

    if [ "$copy_local" = true ]; then
      # ensure bin dir
      [ -d "${path_bin}" ] || mkdir -pv "${path_bin}"

      docker cp "${docker_name}:/usr/local/bin/${name}" "${path_binary}"

      chmod -v +x "${path_binary}"
    fi
  fi
}

set -e

main "$@"
