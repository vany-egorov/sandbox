#!/bin/bash
av \
  --daemonize \
  --foreground \
  --vvv \
  239.255.1.1:5500 \
    -id 1 \
    -name "tears-of-steel-HD" \
    -map all \
      -o file:///tmp/239-255-1-1-5500.ts \
  239.255.1.2:5500 \
    -id 2 \
    -name "tears-of-steel-SD" \
    -metrics "bitrate" \
    -metrics "iat" \
    -metrics "mlr" \
    -map all \
      -o file:///tmp/239-255-1-2-5500-1.ts \
      -o file:///tmp/239-255-1-2-5500-2.ts \
    -map v \
      -o file:///tmp/{{id}}.{{ext}} \
    -map a \
      -o file:///tmp/{{i-u-host}}-{{i-u-port}}-{{pid}}-1.{{ext}} \
      -o file:///tmp/{{i-u-host}}-{{i-u-port}}-{{pid}}-2.{{ext}} \
    -map unk \
      -o file:///tmp/{{i-u-host}}-{{i-u-port}}-{{pid}}-1.dump \
  239.255.1.3:5500 \
    -id 2 \
    -name "big-buck-bunny-HD" \
  \
  http://online.video.rbc.ru/online/rbctv_480p/index.m3u8 \
  \
  -i ../tmp/HD-NatGeoWild.ts \
  \
  -- ../tmp/HD-1.ts
