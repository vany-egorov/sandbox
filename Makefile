SHELL := /bin/bash
USER := $(shell whoami)
REMOTE := "172.16.2.67"

rsync:
	rsync \
		-avz --info=progress2 \
		--rsync-path="mkdir -p /home/${USER}/sandbox/ && rsync" \
		-e "ssh -p 2222" \
		./* \
		--exclude '.git' \
		--exclude 'bin' \
		--exclude 'obj' \
		--exclude 'target' \
		--exclude 'node_modules' \
		--exclude 'bower_components' \
		${USER}@${REMOTE}:/home/${USER}/sandbox/
