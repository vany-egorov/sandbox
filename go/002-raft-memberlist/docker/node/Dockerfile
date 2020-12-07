#syntax=docker/dockerfile:experimental
FROM debian:10 as base

RUN \
  apt-get update && \
  apt-get install -y \
    curl \
    git-core \
    protobuf-compiler

WORKDIR /opt

COPY ./hack/ ./hack/

ENV GOROOT="/opt/go"
ENV GOPATH="/opt/gopath"
ENV PATH="${GOROOT}/bin:${GOPATH}/bin:${PATH}"

RUN /bin/bash \
  ./hack/go-local-install.sh --path /opt

# copy source
WORKDIR /usr/local/raft-memberlist

COPY ./go.mod ./

# install deps
RUN go mod download

COPY ./cmd/ ./cmd
COPY ./internal/ ./internal/
COPY ./pkg/ ./pkg/
COPY ./appversion/ ./appversion/
COPY ./hack/ ./hack/
COPY ./VERSION ./VERSION

# ----------------
FROM base as build

RUN /bin/bash ./hack/build.sh

RUN cp -v /usr/local/raft-memberlist/bin/node /usr/local/bin/node

# ---------------
FROM build as run

ENTRYPOINT ["node"]
