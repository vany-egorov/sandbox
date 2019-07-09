FROM debian:9 as build-rs

RUN apt-get update
RUN apt-get install -y \
  curl \
  musl-tools

RUN curl https://sh.rustup.rs -sSf | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

RUN rustup target add x86_64-unknown-linux-musl

WORKDIR /usr/local/src/nom5

RUN mkdir -v src/
RUN echo "fn main() {println!(\"---\")}" > ./src/main.rs
COPY ./Cargo.toml ./
RUN RUSTFLAGS=-Clinker=musl-gcc cargo \
  build \
  --release \
  --target=x86_64-unknown-linux-musl

RUN rm -rvf ./target/x86_64-unknown-linux-musl/debug/deps/nom5*
RUN rm -rvf ./target/x86_64-unknown-linux-musl/debug/nom5*
RUN rm -rvf ./target/x86_64-unknown-linux-musl/release/deps/nom5*
RUN rm -rvf ./target/x86_64-unknown-linux-musl/release/nom5*
RUN rm -rvf ./src/*

COPY ./src/* ./src/
RUN rustc -V
RUN RUSTFLAGS=-Clinker=musl-gcc cargo \
  build \
  --release \
  --target=x86_64-unknown-linux-musl

# -------------------------------------
FROM alpine:latest

COPY --from=build-rs /usr/local/src/nom5/target/x86_64-unknown-linux-musl/release/nom5 /usr/local/bin/nom5

CMD ["nom5"]
