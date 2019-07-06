FROM debian:9

RUN apt-get update && apt-get install -y \
  curl \
  build-essential

RUN curl https://sh.rustup.rs -sSf | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

COPY ./src/* /usr/local/src/011-udp-mpegts/src/
COPY ./Cargo.toml /usr/local/src/011-udp-mpegts/
WORKDIR /usr/local/src/011-udp-mpegts
CMD ["cargo", "build"]

# WORKDIR /usr/local/src/011-udp-mpegts/target/debug
# CMD ["./udp-mpegts", "--input", "udp://239.255.1.1:5500"]
