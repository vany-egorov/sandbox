#!/bin/bash
source "${BASH_SOURCE%/*}/_path.sh"
source "${path_script}/_compile-proto.sh"
source "${path_script}/_compile-grpc.sh"
source "${path_script}/_mock-generate.sh"

with_bench=false
with_cobertura=false

usage()  {
  cat <<EOF
Usage: $(basename "$0") [name] [options...]
Options:
  -h, --help                        | print help message;
  -b, --bench, --with-bench         | do benchmarks;
  -c, --cobertura, --with-cobertura | do gocover-cobertura
                                    | produce xml with code coverage
                                    | for gitlab ci;
                                    | see see https://docs.gitlab.com/13.4/ee/user/project/merge_requests/test_coverage_visualization.html;
EOF

1>&2;
}


parse_arguments() {
  local options=hbc
  local longopts=help,bench,with-bench,cobertura,with-cobertura

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
      -b | --bench | --with-bench) with_bench=true; shift ;;
      -k | --cobertura | --with-cobertura) with_cobertura=true; shift ;;
      -h | --help) usage; exit 1 ;;
      --) break; ;;
      *) usage; echo ""; echo "unknow option/argument $1; see --help" 1>&2; exit 1; ;;
    esac
  done
}


go_bench() {
  go test \
    ./cmd/... \
    ./internal/... \
    ./appversion/... \
    -bench=. \
    -test.benchmem \
    -test.run=^wontmatchanytest
}

go_test() {
  mkdir -vp "${path_coverage}"

  go test -race \
    ./cmd/... \
    ./internal/... \
    ./appversion/... \
    -cover -coverprofile "${path_cover_out}" \
    -json | tparse -all

  go tool cover -func "${path_cover_out}" | grep total
  go tool cover -html "${path_cover_out}" -o "${path_cover_html}"
}

# see https://docs.gitlab.com/13.4/ee/user/project/merge_requests/test_coverage_visualization.html
gocover_coberture() {
  gocover-cobertura < "${path_cover_out}" > "${path_cobertura_coverage}"
}

patch_gocover_coberture_xml() {
  # replace all filename="github.com/vany-egorov/grpcexample/path/to/file"
  # to filename="/path/to/file"
  #
  # !!! full path relative to the project root !!!
  #
  # From GitLab docs:
  # Note: The Cobertura XML parser currently does not support the
  # sources element and ignores it. It is assumed that the filename
  # of a class element contains the full path relative to the project root.
  sed -e "s;filename=\"github.com/vany-egorov/grpcexample/;filename=\";g" -i "${path_cobertura_coverage}"

  # see https://gitlab.com/gitlab-org/gitlab/-/issues/215747
  #
  # In the event of multiple <source> directives in cobertura.xml,
  # the coverage report fails to parse completely
  # (as it tries to do .each() over a String.
  #
  # leave only single <source></source> tag
  source_count=$(cat ${path_cobertura_coverage} | grep "<source>" | wc -l)
  if [ "$source_count" -gt 1 ]; then
    # remove <source>/go/src</source>
    sed '/<source>\/go\/src<\/source>/d' -i "${path_cobertura_coverage}"
  fi
}

set -e

main() {
  parse_arguments "$@"

  compile_proto
  compile_grpc
  mock_generate

  if [ "$with_bench" = true ]; then
    go_bench
  fi

  go_test

  if [ "$with_cobertura" = true ]; then
    gocover_coberture
    patch_gocover_coberture_xml
  fi
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
  main "$@"
fi
