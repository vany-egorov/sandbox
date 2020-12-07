SHELL := /bin/bash

.DEFAULT: default

default: all-docker

all: build-broker \
	build-worker

all-docker: build-docker-broker \
	build-docker-worker

ensure-bin:
	mkdir -vp ./bin

clean:
	rm -vrf ./bin
	rm -rvf ./coverage
	find . -name "*.pb.go" -type f -delete

# install(download) golang localy to env directory
go-local-install:
	source ./hack/go-local-activate.sh && \
	./hack/go-local-install.sh

build-%: ensure-bin
	./hack/build.sh $*

build-docker-%: ensure-bin
	./hack/build-docker.sh $* --copy-local

.PHONY: test
test:
	./hack/go-test.sh

test-docker:
	./hack/go-test-docker.sh

lint:
	./hack/go-lint.sh

lint-docker:
	./hack/go-lint-docker.sh

run:
	docker-compose up --build broker worker consul
