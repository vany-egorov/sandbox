SHELL := /bin/bash

CXX := g++-6
CXXFLAGS := -g

CC:= gcc-6
# CCFLAGS :=-g -std=c11 -lpthread -lrt
CCFLAGS := -g -std=gnu11 -lpthread -lrt -fPIC -pthread
CCFLAGS += -O2

PATH-PROJECT := $(shell pwd)
OBJS-DIR := $(shell readlink -f $(PATH-PROJECT)/../obj)
BIN-DIR := $(shell readlink -f $(PATH-PROJECT)/../bin)
LIB-DIR := $(shell readlink -f $(PATH-PROJECT)/../lib)

PROJECT-INCLUDES := -I$(PATH-PROJECT)

LDFLAGS += -L/usr/lib64 -ldl


.DEFAULT: default


default: all
all: build


$(OBJS-DIR)/collections-slice.o: ./collections/slice.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/collections-fifo.o: ./collections/fifo.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/collections-slice-test.o: ./collections/slice-test.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/common-opt.o: ./common/opt.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/common-daemonize.o: ./common/daemonize.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/common-container-kind.o: ./common/container-kind.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/common-codec-kind.o: ./common/codec-kind.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/common-fn.o: ./common/fn.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/db-db.o: ./db/db.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/mpegts-mpegts.o: ./mpegts/mpegts.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/mpegts-header.o: ./mpegts/header.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/mpegts-adaption.o: ./mpegts/adaption.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/mpegts-psi.o: ./mpegts/psi.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/mpegts-psi-cat.o: ./mpegts/psi-cat.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/mpegts-psi-nit.o: ./mpegts/psi-nit.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/mpegts-psi-pat.o: ./mpegts/psi-pat.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/mpegts-psi-pmt.o: ./mpegts/psi-pmt.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/mpegts-psi-sdt.o: ./mpegts/psi-sdt.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/mpegts-pes.o: ./mpegts/pes.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/mpegts-es.o: ./mpegts/es.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/h264-h264.o: ./h264/h264.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/h264-nal-sps.o: ./h264/nal-sps.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/h264-nal-pps.o: ./h264/nal-pps.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/h264-nal-aud.o: ./h264/nal-aud.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/h264-nal-sei.o: ./h264/nal-sei.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/h264-nal-slice-idr.o: ./h264/nal-slice-idr.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/h264-bitstream.o: ./h264/bitstream.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/url-url.o: ./url/url.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/url-test.o: ./url/url-test.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/io-io.o: ./io/io.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/io-udp.o: ./io/udp.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/io-file.o: ./io/file.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/io-buf.o: ./io/buf.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/log-level.o: ./log/level.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/log-common.o: ./log/common.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/log-logger.o: ./log/logger.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/log-log.o: ./log/log.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/log-std.o: ./log/std.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/log-std-test.o: ./log/std-test.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/time-time.o: ./time/time.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/time-time-test.o: ./time/time-test.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/va-atom-kind.o: ./va/atom-kind.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/va-parser-worker-read.o: ./va/parser-worker-read.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/va-parser-worker-parse.o: ./va/parser-worker-parse.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/va-parser.o: ./va/parser.c
	$(CC) $(CCFLAGS) -o $@ -c $<


$(OBJS-DIR)/apps-1-config.o: ./apps/1/config.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/apps-1-main.o: ./apps/1/main.c
	$(CC) $(CCFLAGS) -o $@ -c $<


$(OBJS-DIR)/apps-2-main.o: ./apps/2/main.c
	$(CC) $(CCFLAGS) -o $@ -c $<


$(OBJS-DIR)/apps-3-wrkr.o: ./apps/3/wrkr.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-wrkr-cfg.o: ./apps/3/wrkr-cfg.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-input.o: ./apps/3/input.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-input-udp.o: ./apps/3/input-udp.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-input-file.o: ./apps/3/input-file.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-input-logger.o: ./apps/3/input-logger.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-input-build.o: ./apps/3/input-build.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-demuxer.o: ./apps/3/demuxer.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-demuxer-ts.o: ./apps/3/demuxer-ts.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-demuxer-build.o: ./apps/3/demuxer-build.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-stream.o: ./apps/3/stream.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-track.o: ./apps/3/track.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-packet.o: ./apps/3/packet.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-pipeline.o: ./apps/3/pipeline.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-filter.o: ./apps/3/filter.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-filter-logger.o: ./apps/3/filter-logger.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-filter-unknown.o: ./apps/3/filter-unknown.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-filter-metric-b.o: ./apps/3/filter-metric-b.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-filter-h264-parser.o: ./apps/3/filter-h264-parser.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-filter-h264-decoder.o: ./apps/3/filter-h264-decoder.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-filter-ac3-parser.o: ./apps/3/filter-ac3-parser.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-filter-ac3-decoder.o: ./apps/3/filter-ac3-decoder.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-filter-mp2-parser.o: ./apps/3/filter-mp2-parser.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-filter-mp2-decoder.o: ./apps/3/filter-mp2-decoder.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-filter-out-pkt.o: ./apps/3/filter-out-pkt.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-filter-out-pkt-raw.o: ./apps/3/filter-out-pkt-raw.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-signal.o: ./apps/3/signal.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-cfg.o: ./apps/3/cfg.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-map.o: ./apps/3/map.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-main.o: ./apps/3/main.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<


app-1: \
	$(OBJS-DIR)/collections-slice.o \
	$(OBJS-DIR)/collections-fifo.o \
	$(OBJS-DIR)/db-db.o \
	$(OBJS-DIR)/mpegts-mpegts.o \
	$(OBJS-DIR)/mpegts-header.o \
	$(OBJS-DIR)/mpegts-adaption.o \
	$(OBJS-DIR)/mpegts-psi.o \
	$(OBJS-DIR)/mpegts-psi-cat.o \
	$(OBJS-DIR)/mpegts-psi-nit.o \
	$(OBJS-DIR)/mpegts-psi-pat.o \
	$(OBJS-DIR)/mpegts-psi-pmt.o \
	$(OBJS-DIR)/mpegts-psi-sdt.o \
	$(OBJS-DIR)/mpegts-pes.o \
	$(OBJS-DIR)/mpegts-es.o \
	$(OBJS-DIR)/h264-h264.o \
	$(OBJS-DIR)/h264-nal-sps.o \
	$(OBJS-DIR)/h264-nal-pps.o \
	$(OBJS-DIR)/h264-nal-aud.o \
	$(OBJS-DIR)/h264-nal-sei.o \
	$(OBJS-DIR)/h264-nal-slice-idr.o \
	$(OBJS-DIR)/h264-bitstream.o \
	$(OBJS-DIR)/url-url.o \
	$(OBJS-DIR)/io-udp.o \
	$(OBJS-DIR)/io-file.o \
	$(OBJS-DIR)/io-io.o \
	\
	$(OBJS-DIR)/apps-1-config.o \
	$(OBJS-DIR)/apps-1-main.o

	$(CC) $(CCFLAGS) -o $(BIN-DIR)/$@ -static $+ $(LDFLAGS)

app-1-strip: app-1
	strip $(BIN-DIR)/app-1

app-2: \
	$(OBJS-DIR)/collections-slice.o \
	$(OBJS-DIR)/collections-fifo.o \
	$(OBJS-DIR)/db-db.o \
	$(OBJS-DIR)/mpegts-mpegts.o \
	$(OBJS-DIR)/mpegts-header.o \
	$(OBJS-DIR)/mpegts-adaption.o \
	$(OBJS-DIR)/mpegts-psi.o \
	$(OBJS-DIR)/mpegts-psi-cat.o \
	$(OBJS-DIR)/mpegts-psi-nit.o \
	$(OBJS-DIR)/mpegts-psi-pat.o \
	$(OBJS-DIR)/mpegts-psi-pmt.o \
	$(OBJS-DIR)/mpegts-psi-sdt.o \
	$(OBJS-DIR)/mpegts-pes.o \
	$(OBJS-DIR)/mpegts-es.o \
	$(OBJS-DIR)/h264-h264.o \
	$(OBJS-DIR)/h264-nal-sps.o \
	$(OBJS-DIR)/h264-nal-pps.o \
	$(OBJS-DIR)/h264-nal-aud.o \
	$(OBJS-DIR)/h264-nal-sei.o \
	$(OBJS-DIR)/h264-nal-slice-idr.o \
	$(OBJS-DIR)/h264-bitstream.o \
	$(OBJS-DIR)/url-url.o \
	$(OBJS-DIR)/io-udp.o \
	$(OBJS-DIR)/io-file.o \
	$(OBJS-DIR)/io-io.o \
	$(OBJS-DIR)/va-atom-kind.o \
	$(OBJS-DIR)/va-parser.o \
	$(OBJS-DIR)/va-parser-worker-read.o \
	$(OBJS-DIR)/va-parser-worker-parse.o \
	\
	$(OBJS-DIR)/apps-2-main.o

	$(CC) $(CCFLAGS) -o $(BIN-DIR)/$@ -static $+ $(LDFLAGS)

app-2-strip: app-2
	strip $(BIN-DIR)/app-2

app-3: \
	$(OBJS-DIR)/url-url.o \
	$(OBJS-DIR)/io-io.o \
	$(OBJS-DIR)/io-udp.o \
	$(OBJS-DIR)/io-file.o \
	$(OBJS-DIR)/io-buf.o \
	\
	$(OBJS-DIR)/collections-fifo.o \
	$(OBJS-DIR)/collections-slice.o \
	\
	$(OBJS-DIR)/common-opt.o \
	$(OBJS-DIR)/common-container-kind.o \
	$(OBJS-DIR)/common-codec-kind.o \
	$(OBJS-DIR)/common-fn.o \
	$(OBJS-DIR)/common-daemonize.o \
	\
	$(OBJS-DIR)/log-level.o \
	$(OBJS-DIR)/log-common.o \
	$(OBJS-DIR)/log-log.o \
	$(OBJS-DIR)/log-logger.o \
	$(OBJS-DIR)/log-std.o \
	\
	$(OBJS-DIR)/apps-3-cfg.o \
	$(OBJS-DIR)/apps-3-map.o \
	$(OBJS-DIR)/apps-3-signal.o \
	$(OBJS-DIR)/apps-3-wrkr.o \
	$(OBJS-DIR)/apps-3-wrkr-cfg.o \
	$(OBJS-DIR)/apps-3-input.o \
	$(OBJS-DIR)/apps-3-input-udp.o \
	$(OBJS-DIR)/apps-3-input-file.o \
	$(OBJS-DIR)/apps-3-input-logger.o \
	$(OBJS-DIR)/apps-3-input-build.o \
	$(OBJS-DIR)/apps-3-demuxer.o \
	$(OBJS-DIR)/apps-3-demuxer-ts.o \
	$(OBJS-DIR)/apps-3-demuxer-build.o \
	$(OBJS-DIR)/apps-3-stream.o \
	$(OBJS-DIR)/apps-3-track.o \
	$(OBJS-DIR)/apps-3-packet.o \
	$(OBJS-DIR)/apps-3-pipeline.o \
	$(OBJS-DIR)/apps-3-filter.o \
	$(OBJS-DIR)/apps-3-filter-logger.o \
	$(OBJS-DIR)/apps-3-filter-unknown.o \
	$(OBJS-DIR)/apps-3-filter-metric-b.o \
	$(OBJS-DIR)/apps-3-filter-h264-parser.o \
	$(OBJS-DIR)/apps-3-filter-h264-decoder.o \
	$(OBJS-DIR)/apps-3-filter-ac3-parser.o \
	$(OBJS-DIR)/apps-3-filter-ac3-decoder.o \
	$(OBJS-DIR)/apps-3-filter-mp2-parser.o \
	$(OBJS-DIR)/apps-3-filter-mp2-decoder.o \
	$(OBJS-DIR)/apps-3-filter-out-pkt.o \
	$(OBJS-DIR)/apps-3-filter-out-pkt-raw.o \
	\
	$(OBJS-DIR)/mpegts-mpegts.o \
	$(OBJS-DIR)/mpegts-header.o \
	$(OBJS-DIR)/mpegts-adaption.o \
	$(OBJS-DIR)/mpegts-psi.o \
	$(OBJS-DIR)/mpegts-psi-cat.o \
	$(OBJS-DIR)/mpegts-psi-nit.o \
	$(OBJS-DIR)/mpegts-psi-pat.o \
	$(OBJS-DIR)/mpegts-psi-pmt.o \
	$(OBJS-DIR)/mpegts-psi-sdt.o \
	$(OBJS-DIR)/mpegts-pes.o \
	$(OBJS-DIR)/mpegts-es.o \
	\
	$(OBJS-DIR)/h264-h264.o \
	$(OBJS-DIR)/h264-nal-sps.o \
	$(OBJS-DIR)/h264-nal-pps.o \
	$(OBJS-DIR)/h264-nal-aud.o \
	$(OBJS-DIR)/h264-nal-sei.o \
	$(OBJS-DIR)/h264-nal-slice-idr.o \
	$(OBJS-DIR)/h264-bitstream.o \
	\
	$(OBJS-DIR)/apps-3-main.o

	$(CC) $(CCFLAGS) -o $(BIN-DIR)/$@ -static $+ $(LDFLAGS)

app-3-strip: app-3
	strip $(BIN-DIR)/app-3

static: \
	$(OBJS-DIR)/collections-slice.o \
	$(OBJS-DIR)/collections-fifo.o \
	$(OBJS-DIR)/db-db.o \
	$(OBJS-DIR)/mpegts-mpegts.o \
	$(OBJS-DIR)/mpegts-header.o \
	$(OBJS-DIR)/mpegts-adaption.o \
	$(OBJS-DIR)/mpegts-psi.o \
	$(OBJS-DIR)/mpegts-psi-cat.o \
	$(OBJS-DIR)/mpegts-psi-nit.o \
	$(OBJS-DIR)/mpegts-psi-pat.o \
	$(OBJS-DIR)/mpegts-psi-pmt.o \
	$(OBJS-DIR)/mpegts-psi-sdt.o \
	$(OBJS-DIR)/mpegts-pes.o \
	$(OBJS-DIR)/mpegts-es.o \
	$(OBJS-DIR)/h264-h264.o \
	$(OBJS-DIR)/h264-nal-sps.o \
	$(OBJS-DIR)/h264-nal-pps.o \
	$(OBJS-DIR)/h264-nal-aud.o \
	$(OBJS-DIR)/h264-nal-sei.o \
	$(OBJS-DIR)/h264-nal-slice-idr.o \
	$(OBJS-DIR)/h264-bitstream.o \
	$(OBJS-DIR)/url-url.o \
	$(OBJS-DIR)/io-udp.o \
	$(OBJS-DIR)/io-file.o \
	$(OBJS-DIR)/io-io.o \
	$(OBJS-DIR)/va-atom-kind.o \
	$(OBJS-DIR)/va-parser.o \
	$(OBJS-DIR)/va-parser-worker-read.o \
	$(OBJS-DIR)/va-parser-worker-parse.o
	ar -cvq $(LIB-DIR)/libva.a $+

dynamic: \
	$(OBJS-DIR)/collections-slice.o \
	$(OBJS-DIR)/collections-fifo.o \
	$(OBJS-DIR)/db-db.o \
	$(OBJS-DIR)/mpegts-mpegts.o \
	$(OBJS-DIR)/mpegts-header.o \
	$(OBJS-DIR)/mpegts-adaption.o \
	$(OBJS-DIR)/mpegts-psi.o \
	$(OBJS-DIR)/mpegts-psi-cat.o \
	$(OBJS-DIR)/mpegts-psi-nit.o \
	$(OBJS-DIR)/mpegts-psi-pat.o \
	$(OBJS-DIR)/mpegts-psi-pmt.o \
	$(OBJS-DIR)/mpegts-psi-sdt.o \
	$(OBJS-DIR)/mpegts-pes.o \
	$(OBJS-DIR)/mpegts-es.o \
	$(OBJS-DIR)/h264-h264.o \
	$(OBJS-DIR)/h264-nal-sps.o \
	$(OBJS-DIR)/h264-nal-pps.o \
	$(OBJS-DIR)/h264-nal-aud.o \
	$(OBJS-DIR)/h264-nal-sei.o \
	$(OBJS-DIR)/h264-nal-slice-idr.o \
	$(OBJS-DIR)/h264-bitstream.o \
	$(OBJS-DIR)/url-url.o \
	$(OBJS-DIR)/io-udp.o \
	$(OBJS-DIR)/io-file.o \
	$(OBJS-DIR)/io-io.o \
	$(OBJS-DIR)/va-atom-kind.o \
	$(OBJS-DIR)/va-parser.o \
	$(OBJS-DIR)/va-parser-worker-read.o \
	$(OBJS-DIR)/va-parser-worker-parse.o
	$(CC) $+ -shared -o $(LIB-DIR)/libva.so

log-std-test: \
	$(OBJS-DIR)/log-level.o \
	$(OBJS-DIR)/log-common.o \
	$(OBJS-DIR)/log-logger.o \
	$(OBJS-DIR)/log-std.o \
	$(OBJS-DIR)/log-std-test.o
	$(CC) $(CCFLAGS) -o $(BIN-DIR)/$@ $+ $(LDFLAGS)

url-test: \
	$(OBJS-DIR)/url-url.o \
	$(OBJS-DIR)/url-test.o
	$(CC) $(CCFLAGS) -o $(BIN-DIR)/$@ $+ $(LDFLAGS)

time-test: \
	$(OBJS-DIR)/time-time.o \
	$(OBJS-DIR)/time-time-test.o
	$(CC) $(CCFLAGS) -o $(BIN-DIR)/$@ $+ $(LDFLAGS)

collections-slice-test: \
	$(OBJS-DIR)/collections-slice.o \
	$(OBJS-DIR)/collections-slice-test.o
	$(CC) $(CCFLAGS) -o $(BIN-DIR)/$@ $+ $(LDFLAGS)

ensure-dirs:
	mkdir -p $(OBJS-DIR)
	mkdir -p $(BIN-DIR)
	mkdir -p $(LIB-DIR)

build: ensure-dirs static dynamic app-1 app-2 app-3 collections-slice-test url-test log-std-test time-test app-1-strip app-2-strip app-3-strip

clean:
	rm -rvf $(OBJS-DIR)/*.o
	rm -rvf $(OBJS-DIR)/*.so
	rm -rvf $(OBJS-DIR)/*.a
	rm -rvf $(OBJS-DIR)/app-1
	rm -rvf $(OBJS-DIR)/app-2
	rm -rvf $(OBJS-DIR)/app-3
	rm -rvf $(OBJS-DIR)/collections-slice-test
	rm -rvf $(OBJS-DIR)/url-test
	rm -rvf $(OBJS-DIR)/time-test
	rm -rvf $(OBJS-DIR)/log-std-test

	rm -rvf $(BIN-DIR)/*.o
	rm -rvf $(BIN-DIR)/*.so
	rm -rvf $(BIN-DIR)/*.a
	rm -rvf $(BIN-DIR)/app-1
	rm -rvf $(BIN-DIR)/app-2
	rm -rvf $(BIN-DIR)/app-3
	rm -rvf $(BIN-DIR)/collections-slice-test
	rm -rvf $(BIN-DIR)/url-test
	rm -rvf $(BIN-DIR)/time-test
	rm -rvf $(BIN-DIR)/log-std-test

	rm -rvf $(LIB-DIR)/*.o
	rm -rvf $(LIB-DIR)/*.so
	rm -rvf $(LIB-DIR)/*.a
	rm -rvf $(LIB-DIR)/app-1
	rm -rvf $(LIB-DIR)/app-2
	rm -rvf $(LIB-DIR)/app-3
	rm -rvf $(LIB-DIR)/collections-slice-test
	rm -rvf $(LIB-DIR)/url-test
	rm -rvf $(LIB-DIR)/time-test
	rm -rvf $(LIB-DIR)/log-std-test

# tsplay ../tmp/HD-1.ts 239.255.1.1:5500 -loop
# tsplay ../tmp/HD-NatGeoWild.ts 239.255.1.2:5500 -loop
run:
	# $(OBJS-DIR)/app 2>&1 >out.log
	$(OBJS-DIR)/app-1 -i 239.1.1.1:5500
