SHELL := /bin/bash

.DEFAULT: default

default: all-docker

all: build \
	build-node

all-docker: build-docker \
	build-docker-node

ensure-bin:
	mkdir -vp ./bin

clean:
	rm -vrf ./bin
	find . -name "*.pb.go" -type f -delete

# install(download) golang localy to env directory
go-local-install:
	source ./hack/go-local-activate.sh && \
	./hack/go-local-install.sh


build: ensure-bin
	./hack/build.sh

# e.g. build-api
#
# "api" will be captured as %
# and can be accessible via $*
build-%: ensure-bin
	./hack/build.sh $*


build-docker: ensure-bin
	./hack/build-docker.sh --copy-local

# e.g. build-docker-api
#
# "api" will be captured as %
# and can be accessible via $*
build-docker-%: ensure-bin
	./hack/build-docker.sh $* --copy-local

run:
	docker-compose -f ./docker/docker-compose.yml up --build
