CXX := g++
CXXFLAGS := -g

CC:= gcc
# CCFLAGS :=-g -std=c11 -lpthread -lrt
CCFLAGS := -g -lpthread -lrt -fPIC

LDFLAGS += -L/usr/lib64 -ldl -lstdc++

OBJS_DIR := ../obj
BIN_DIR := ../bin
LIB_DIR := ../lib


$(OBJS_DIR)/collections-slice.o: ./collections/slice.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/collections-fifo.o: ./collections/fifo.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/collections-slice-test.o: ./collections/slice-test.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/db-db.o: ./db/db.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/mpegts-mpegts.o: ./mpegts/mpegts.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/mpegts-header.o: ./mpegts/header.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/mpegts-adaption.o: ./mpegts/adaption.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/mpegts-psi.o: ./mpegts/psi.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/mpegts-psi-cat.o: ./mpegts/psi-cat.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/mpegts-psi-nit.o: ./mpegts/psi-nit.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/mpegts-psi-pat.o: ./mpegts/psi-pat.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/mpegts-psi-pmt.o: ./mpegts/psi-pmt.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/mpegts-psi-sdt.o: ./mpegts/psi-sdt.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/mpegts-pes.o: ./mpegts/pes.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/mpegts-es.o: ./mpegts/es.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/h264-h264.o: ./h264/h264.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/h264-nal-sps.o: ./h264/nal-sps.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/h264-nal-pps.o: ./h264/nal-pps.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/h264-nal-aud.o: ./h264/nal-aud.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/h264-nal-sei.o: ./h264/nal-sei.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/h264-nal-slice-idr.o: ./h264/nal-slice-idr.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/h264-bitstream.o: ./h264/bitstream.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/url-url.o: ./url/url.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/io-io.o: ./io/io.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/io-udp.o: ./io/udp.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/io-file.o: ./io/file.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/va-atom-kind.o: ./va/atom-kind.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/va-parser-worker-read.o: ./va/parser-worker-read.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/va-parser-worker-parse.o: ./va/parser-worker-parse.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/va-parser.o: ./va/parser.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/config.o: ./config.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/main-1.o: ./main-1.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/main-2.o: ./main-2.c
	$(CC) $(CCFLAGS) -o $@ -c $<


main-1: \
	$(OBJS_DIR)/collections-slice.o \
	$(OBJS_DIR)/collections-fifo.o \
	$(OBJS_DIR)/db-db.o \
	$(OBJS_DIR)/mpegts-mpegts.o \
	$(OBJS_DIR)/mpegts-header.o \
	$(OBJS_DIR)/mpegts-adaption.o \
	$(OBJS_DIR)/mpegts-psi.o \
	$(OBJS_DIR)/mpegts-psi-cat.o \
	$(OBJS_DIR)/mpegts-psi-nit.o \
	$(OBJS_DIR)/mpegts-psi-pat.o \
	$(OBJS_DIR)/mpegts-psi-pmt.o \
	$(OBJS_DIR)/mpegts-psi-sdt.o \
	$(OBJS_DIR)/mpegts-pes.o \
	$(OBJS_DIR)/mpegts-es.o \
	$(OBJS_DIR)/h264-h264.o \
	$(OBJS_DIR)/h264-nal-sps.o \
	$(OBJS_DIR)/h264-nal-pps.o \
	$(OBJS_DIR)/h264-nal-aud.o \
	$(OBJS_DIR)/h264-nal-sei.o \
	$(OBJS_DIR)/h264-nal-slice-idr.o \
	$(OBJS_DIR)/h264-bitstream.o \
	$(OBJS_DIR)/url-url.o \
	$(OBJS_DIR)/io-udp.o \
	$(OBJS_DIR)/io-file.o \
	$(OBJS_DIR)/io-io.o \
	$(OBJS_DIR)/config.o \
	$(OBJS_DIR)/main-1.o
	$(CC) $(CCFLAGS) -o $(BIN_DIR)/$@ $+ $(LDFLAGS)

main-2: \
	$(OBJS_DIR)/collections-slice.o \
	$(OBJS_DIR)/collections-fifo.o \
	$(OBJS_DIR)/db-db.o \
	$(OBJS_DIR)/mpegts-mpegts.o \
	$(OBJS_DIR)/mpegts-header.o \
	$(OBJS_DIR)/mpegts-adaption.o \
	$(OBJS_DIR)/mpegts-psi.o \
	$(OBJS_DIR)/mpegts-psi-cat.o \
	$(OBJS_DIR)/mpegts-psi-nit.o \
	$(OBJS_DIR)/mpegts-psi-pat.o \
	$(OBJS_DIR)/mpegts-psi-pmt.o \
	$(OBJS_DIR)/mpegts-psi-sdt.o \
	$(OBJS_DIR)/mpegts-pes.o \
	$(OBJS_DIR)/mpegts-es.o \
	$(OBJS_DIR)/h264-h264.o \
	$(OBJS_DIR)/h264-nal-sps.o \
	$(OBJS_DIR)/h264-nal-pps.o \
	$(OBJS_DIR)/h264-nal-aud.o \
	$(OBJS_DIR)/h264-nal-sei.o \
	$(OBJS_DIR)/h264-nal-slice-idr.o \
	$(OBJS_DIR)/h264-bitstream.o \
	$(OBJS_DIR)/url-url.o \
	$(OBJS_DIR)/io-udp.o \
	$(OBJS_DIR)/io-file.o \
	$(OBJS_DIR)/io-io.o \
	$(OBJS_DIR)/va-atom-kind.o \
	$(OBJS_DIR)/va-parser.o \
	$(OBJS_DIR)/va-parser-worker-read.o \
	$(OBJS_DIR)/va-parser-worker-parse.o \
	$(OBJS_DIR)/main-2.o
	$(CC) $(CCFLAGS) -o $(BIN_DIR)/$@ $+ $(LDFLAGS)

static: \
	$(OBJS_DIR)/collections-slice.o \
	$(OBJS_DIR)/collections-fifo.o \
	$(OBJS_DIR)/db-db.o \
	$(OBJS_DIR)/mpegts-mpegts.o \
	$(OBJS_DIR)/mpegts-header.o \
	$(OBJS_DIR)/mpegts-adaption.o \
	$(OBJS_DIR)/mpegts-psi.o \
	$(OBJS_DIR)/mpegts-psi-cat.o \
	$(OBJS_DIR)/mpegts-psi-nit.o \
	$(OBJS_DIR)/mpegts-psi-pat.o \
	$(OBJS_DIR)/mpegts-psi-pmt.o \
	$(OBJS_DIR)/mpegts-psi-sdt.o \
	$(OBJS_DIR)/mpegts-pes.o \
	$(OBJS_DIR)/mpegts-es.o \
	$(OBJS_DIR)/h264-h264.o \
	$(OBJS_DIR)/h264-nal-sps.o \
	$(OBJS_DIR)/h264-nal-pps.o \
	$(OBJS_DIR)/h264-nal-aud.o \
	$(OBJS_DIR)/h264-nal-sei.o \
	$(OBJS_DIR)/h264-nal-slice-idr.o \
	$(OBJS_DIR)/h264-bitstream.o \
	$(OBJS_DIR)/url-url.o \
	$(OBJS_DIR)/io-udp.o \
	$(OBJS_DIR)/io-file.o \
	$(OBJS_DIR)/io-io.o \
	$(OBJS_DIR)/va-atom-kind.o \
	$(OBJS_DIR)/va-parser.o \
	$(OBJS_DIR)/va-parser-worker-read.o \
	$(OBJS_DIR)/va-parser-worker-parse.o
	ar -cvq $(LIB_DIR)/libva.a $+

dynamic: \
	$(OBJS_DIR)/collections-slice.o \
	$(OBJS_DIR)/collections-fifo.o \
	$(OBJS_DIR)/db-db.o \
	$(OBJS_DIR)/mpegts-mpegts.o \
	$(OBJS_DIR)/mpegts-header.o \
	$(OBJS_DIR)/mpegts-adaption.o \
	$(OBJS_DIR)/mpegts-psi.o \
	$(OBJS_DIR)/mpegts-psi-cat.o \
	$(OBJS_DIR)/mpegts-psi-nit.o \
	$(OBJS_DIR)/mpegts-psi-pat.o \
	$(OBJS_DIR)/mpegts-psi-pmt.o \
	$(OBJS_DIR)/mpegts-psi-sdt.o \
	$(OBJS_DIR)/mpegts-pes.o \
	$(OBJS_DIR)/mpegts-es.o \
	$(OBJS_DIR)/h264-h264.o \
	$(OBJS_DIR)/h264-nal-sps.o \
	$(OBJS_DIR)/h264-nal-pps.o \
	$(OBJS_DIR)/h264-nal-aud.o \
	$(OBJS_DIR)/h264-nal-sei.o \
	$(OBJS_DIR)/h264-nal-slice-idr.o \
	$(OBJS_DIR)/h264-bitstream.o \
	$(OBJS_DIR)/url-url.o \
	$(OBJS_DIR)/io-udp.o \
	$(OBJS_DIR)/io-file.o \
	$(OBJS_DIR)/io-io.o \
	$(OBJS_DIR)/va-atom-kind.o \
	$(OBJS_DIR)/va-parser.o \
	$(OBJS_DIR)/va-parser-worker-read.o \
	$(OBJS_DIR)/va-parser-worker-parse.o
	$(CC) $+ -shared -o $(LIB_DIR)/libva.so

collections-slice-test: \
	$(OBJS_DIR)/collections-slice.o \
	$(OBJS_DIR)/collections-slice-test.o
	$(CC) $(CCFLAGS) -o $(BIN_DIR)/$@ $+ $(LDFLAGS)

ensure-dirs:
	mkdir -p $(OBJS_DIR)
	mkdir -p $(BIN_DIR)
	mkdir -p $(LIB_DIR)

build: ensure-dirs static dynamic main-1 main-2 collections-slice-test

clean:
	rm -rvf $(OBJS_DIR)/*.o
	rm -rvf $(OBJS_DIR)/*.so
	rm -rvf $(OBJS_DIR)/*.a
	rm -rvf $(OBJS_DIR)/main-1
	rm -rvf $(OBJS_DIR)/main-2
	rm -rvf $(OBJS_DIR)/collections-slice-test

	rm -rvf $(BIN_DIR)/*.o
	rm -rvf $(BIN_DIR)/*.so
	rm -rvf $(BIN_DIR)/*.a
	rm -rvf $(BIN_DIR)/main-1
	rm -rvf $(BIN_DIR)/main-2
	rm -rvf $(BIN_DIR)/collections-slice-test

	rm -rvf $(LIB_DIR)/*.o
	rm -rvf $(LIB_DIR)/*.so
	rm -rvf $(LIB_DIR)/*.a
	rm -rvf $(LIB_DIR)/main-1
	rm -rvf $(LIB_DIR)/main-2
	rm -rvf $(LIB_DIR)/collections-slice-test

# tsplay ../tmp/hd.ts 239.1.1.1:5500 -loop
run:
	# $(OBJS_DIR)/main 2>&1 >out.log
	$(OBJS_DIR)/main -i 239.1.1.1:5500
