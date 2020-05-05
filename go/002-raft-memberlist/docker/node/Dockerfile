#syntax=docker/dockerfile:experimental
FROM debian:10 as base

RUN \
  apt-get update && \
  apt-get install -y \
    curl \
    git-core \
    protobuf-compiler

# install golang
WORKDIR /opt

COPY ./hack/ ./hack/

# -----------------------------------------------------------------------------
FROM base as build
