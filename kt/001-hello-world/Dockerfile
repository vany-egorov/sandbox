FROM debian:10

RUN apt-get update && apt-get -y --no-install-recommends install \
  bash \
  unzip \
  zip \
  curl \
  && \
  rm -rvf /var/lib/apt/lists/*

RUN curl -s "https://get.sdkman.io" | bash

RUN source "$HOME/.sdkman/bin/sdkman-init.sh" \
  && sdk install kotlin

WORKDIR /usr/local/app
COPY ./main.kt ./

RUN kotlinc main.kt
