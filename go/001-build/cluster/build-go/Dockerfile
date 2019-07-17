FROM debian:10 as build-go

ENV GO_VERSION="1.12.7"
ENV GO_LDFLAGS="-s -w"
ENV GOOS="linux"
ENV GOARCH="amd64"

RUN apt-get update
RUN apt-get install -y \
  curl \
  git-core

# <golang>
WORKDIR /opt
RUN curl -O "https://storage.googleapis.com/golang/go${GO_VERSION}.linux-amd64.tar.gz"
RUN mv ./go*tar.gz ./go.tar.gz
RUN tar -xf ./go.tar.gz
RUN rm -rf ./go.tar.gz
# </golang>

ENV GOROOT="/opt/go"
ENV PATH="${GOROOT}/bin:${PATH}"

WORKDIR /usr/local/src/go-build
COPY ./src/go.mod ./
RUN go mod download
COPY ./src/ ./
RUN rm -rvf ./src/go.sum
RUN GOOS=${GOOS} GOARCH=${GOARCH} \
  go build \
    -v \
    -ldflags "${GO_LDFLAGS}" -o ./go-build ./main.go

# -------------------------------------
FROM debian:10

COPY --from=build-go /usr/local/src/go-build/go-build /usr/local/bin/go-build

ENTRYPOINT ["/usr/local/bin/go-build"]
