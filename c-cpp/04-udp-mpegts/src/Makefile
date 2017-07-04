SHELL := /bin/bash

CXX := g++
CXXFLAGS := -g

CC:= gcc
# CCFLAGS :=-g -std=c11 -lpthread -lrt
CCFLAGS := -g -std=gnu11 -lpthread -lrt -fPIC -pthread

PATH-PROJECT := $(shell pwd)
OBJS-DIR := $(shell realpath $(PATH-PROJECT)/../obj)
BIN-DIR := $(shell realpath $(PATH-PROJECT)/../bin)
LIB-DIR := $(shell realpath $(PATH-PROJECT)/../lib)

PROJECT-INCLUDES := -I$(PATH-PROJECT)

LDFLAGS += -L/usr/lib64 -ldl -lstdc++


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

$(OBJS-DIR)/io-io.o: ./io/io.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/io-udp.o: ./io/udp.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS-DIR)/io-file.o: ./io/file.c
	$(CC) $(CCFLAGS) -o $@ -c $<

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

$(OBJS-DIR)/apps-3-input.o: ./apps/3/input.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-input-udp.o: ./apps/3/input-udp.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-input-file.o: ./apps/3/input-file.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-signal.o: ./apps/3/signal.c
	$(CC) $(CCFLAGS) $(PROJECT-INCLUDES) -o $@ -c $<

$(OBJS-DIR)/apps-3-cfg.o: ./apps/3/cfg.c
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

	$(CC) $(CCFLAGS) -o $(BIN-DIR)/$@ $+ $(LDFLAGS)

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

	$(CC) $(CCFLAGS) -o $(BIN-DIR)/$@ $+ $(LDFLAGS)

app-3: \
	$(OBJS-DIR)/url-url.o \
	$(OBJS-DIR)/io-udp.o \
	$(OBJS-DIR)/io-file.o \
	$(OBJS-DIR)/collections-fifo.o \
	$(OBJS-DIR)/collections-slice.o \
	$(OBJS-DIR)/common-opt.o \
	\
	$(OBJS-DIR)/apps-3-cfg.o \
	$(OBJS-DIR)/apps-3-signal.o \
	$(OBJS-DIR)/apps-3-wrkr.o \
	$(OBJS-DIR)/apps-3-input.o \
	$(OBJS-DIR)/apps-3-input-udp.o \
	$(OBJS-DIR)/apps-3-input-file.o \
	\
	$(OBJS-DIR)/apps-3-main.o

	$(CC) $(CCFLAGS) -o $(BIN-DIR)/$@ $+ $(LDFLAGS)

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

collections-slice-test: \
	$(OBJS-DIR)/collections-slice.o \
	$(OBJS-DIR)/collections-slice-test.o
	$(CC) $(CCFLAGS) -o $(BIN-DIR)/$@ $+ $(LDFLAGS)

ensure-dirs:
	mkdir -p $(OBJS-DIR)
	mkdir -p $(BIN-DIR)
	mkdir -p $(LIB-DIR)

build: ensure-dirs static dynamic app-1 app-2 app-3 collections-slice-test

clean:
	rm -rvf $(OBJS-DIR)/*.o
	rm -rvf $(OBJS-DIR)/*.so
	rm -rvf $(OBJS-DIR)/*.a
	rm -rvf $(OBJS-DIR)/app-1
	rm -rvf $(OBJS-DIR)/app-2
	rm -rvf $(OBJS-DIR)/app-3
	rm -rvf $(OBJS-DIR)/collections-slice-test

	rm -rvf $(BIN-DIR)/*.o
	rm -rvf $(BIN-DIR)/*.so
	rm -rvf $(BIN-DIR)/*.a
	rm -rvf $(BIN-DIR)/app-1
	rm -rvf $(BIN-DIR)/app-2
	rm -rvf $(BIN-DIR)/app-3
	rm -rvf $(BIN-DIR)/collections-slice-test

	rm -rvf $(LIB-DIR)/*.o
	rm -rvf $(LIB-DIR)/*.so
	rm -rvf $(LIB-DIR)/*.a
	rm -rvf $(LIB-DIR)/app-1
	rm -rvf $(LIB-DIR)/app-2
	rm -rvf $(LIB-DIR)/app-3
	rm -rvf $(LIB-DIR)/collections-slice-test

# tsplay ../tmp/hd.ts 239.1.1.1:5500 -loop
run:
	# $(OBJS-DIR)/app 2>&1 >out.log
	$(OBJS-DIR)/app-1 -i 239.1.1.1:5500
