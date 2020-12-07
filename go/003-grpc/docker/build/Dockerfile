FROM golang:buster as base

RUN \
  apt-get update && \
  apt-get install -y \
    protobuf-compiler

ENV GO111MODULE=on

WORKDIR /usr/local/app

COPY ./hack/golangci-lint-version.sh ./hack/golangci-lint-version.sh
COPY ./hack/go-get.sh ./hack/go-get.sh
COPY ./go.mod ./
COPY ./go.sum ./

RUN /bin/bash ./hack/go-get.sh
# install deps
RUN go mod download

COPY ./hack/ ./hack/
COPY ./cmd/ ./cmd
COPY ./internal/ ./internal
COPY ./appversion/ ./appversion/
COPY ./VERSION ./VERSION

# -----------------------------------------------------------------------------
FROM base as lint

CMD ["./hack/go-lint.sh"]
ENTRYPOINT ["/bin/bash"]

# -----------------------------------------------------------------------------
FROM base as test

CMD ["./hack/go-test.sh", "--with-bench", "--with-cobertura"]
ENTRYPOINT ["/bin/bash"]

# -----------------------------------------------------------------------------
FROM base as build-broker

RUN /bin/bash ./hack/build.sh broker

RUN cp -v /usr/local/app/bin/broker /usr/local/bin/broker

FROM base as build-worker

RUN /bin/bash ./hack/build.sh worker

RUN cp -v /usr/local/app/bin/worker /usr/local/bin/worker

# -----------------------------------------------------------------------------
FROM debian:10 as broker

COPY --from=build-broker /usr/local/bin/broker /usr/local/bin/broker

CMD ["--help"]
ENTRYPOINT ["broker"]

FROM debian:10 as worker

COPY --from=build-worker /usr/local/bin/worker /usr/local/bin/worker
RUN ls -lah /usr/local/bin/worker

CMD ["--help"]
ENTRYPOINT ["worker"]
