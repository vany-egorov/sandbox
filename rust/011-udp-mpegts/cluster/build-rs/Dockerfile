FROM debian:9 as build-rs

RUN apt-get update
RUN apt-get install -y \
  curl \
  musl-tools

RUN curl https://sh.rustup.rs -sSf | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

RUN rustup target add x86_64-unknown-linux-musl

WORKDIR /usr/local/src/udp-mpegts

RUN mkdir -v src/
RUN echo "fn main() {println!(\"---\")}" > ./src/main.rs
COPY ./Cargo.toml ./
RUN RUSTFLAGS=-Clinker=musl-gcc cargo \
  build \
  --target=x86_64-unknown-linux-musl

RUN rm -rvf ./target/x86_64-unknown-linux-musl/debug/deps/udp-mpegts*
RUN rm -rvf ./target/x86_64-unknown-linux-musl/debug/udp-mpegts*
RUN rm -rvf ./src/*

COPY ./src/* ./src/
RUN RUSTFLAGS=-Clinker=musl-gcc cargo \
  build \
  --target=x86_64-unknown-linux-musl

# -------------------------------------
FROM alpine:latest

COPY --from=build-rs /usr/local/src/udp-mpegts/target/x86_64-unknown-linux-musl/debug/udp-mpegts /usr/local/bin/udp-mpegts

CMD ["udp-mpegts", "--input", "udp://239.255.1.1:5500"]
