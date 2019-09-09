FROM debian:10 as build-rs

RUN apt-get update
RUN apt-get install -y \
  curl \
  musl-tools

RUN curl https://sh.rustup.rs -sSf | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

RUN rustup toolchain install nightly
RUN rustup target add x86_64-unknown-linux-musl
RUN rustup target add --toolchain nightly x86_64-unknown-linux-musl

WORKDIR /usr/local/src/tokio-signal

RUN mkdir -pv src/
RUN echo "fn main() {println!(\"---\")}" > ./src/main.rs
COPY ./Cargo.toml ./
RUN RUSTFLAGS=-Clinker=musl-gcc cargo \
  +nightly \
  build \
  --release \
  --verbose \
  --target=x86_64-unknown-linux-musl

RUN rm -rvf ./src/*

COPY ./src/ ./src/
RUN RUSTFLAGS=-Clinker=musl-gcc cargo \
  +nightly \
  build \
  --release \
  --verbose \
  --target=x86_64-unknown-linux-musl

# -------------------------------------
FROM alpine:latest

COPY --from=build-rs /usr/local/src/tokio-signal/target/x86_64-unknown-linux-musl/release/tokio-signal /usr/local/bin/tokio-signal
