#!/bin/bash
docker-compose -f ./cluster/docker-compose.yml up --build --scale "ha-eta-node=5"
