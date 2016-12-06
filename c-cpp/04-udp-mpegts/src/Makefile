CXX := g++
CXXFLAGS := -g

CC:= gcc
# CCFLAGS :=-g -std=c11 -lpthread -lrt
CCFLAGS := -g -lpthread -lrt

LDFLAGS += -L/usr/lib64 -ldl -lstdc++

OBJECTS_DIRS := ../bin


$(OBJECTS_DIRS)/collections-slice.o: ./collections/slice.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/collections-fifo.o: ./collections/fifo.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/collections-slice-test.o: ./collections/slice-test.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/db-db.o: ./db/db.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/mpegts-mpegts.o: ./mpegts/mpegts.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/mpegts-header.o: ./mpegts/header.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/mpegts-adaption.o: ./mpegts/adaption.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/mpegts-psi.o: ./mpegts/psi.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/mpegts-psi-cat.o: ./mpegts/psi-cat.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/mpegts-psi-nit.o: ./mpegts/psi-nit.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/mpegts-psi-pat.o: ./mpegts/psi-pat.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/mpegts-psi-pmt.o: ./mpegts/psi-pmt.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/mpegts-psi-sdt.o: ./mpegts/psi-sdt.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/mpegts-pes.o: ./mpegts/pes.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/mpegts-es.o: ./mpegts/es.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/h264-h264.o: ./h264/h264.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/h264-nal-sps.o: ./h264/nal-sps.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/h264-nal-pps.o: ./h264/nal-pps.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/h264-nal-aud.o: ./h264/nal-aud.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/h264-nal-sei.o: ./h264/nal-sei.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/h264-nal-slice-idr.o: ./h264/nal-slice-idr.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/h264-bitstream.o: ./h264/bitstream.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/url-url.o: ./url/url.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/io-io.o: ./io/io.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/io-udp.o: ./io/udp.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/io-file.o: ./io/file.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/va-parser.o: ./va/parser.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/config.o: ./config.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/main-1.o: ./main-1.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/main-2.o: ./main-2.c
	$(CC) $(CCFLAGS) -o $@ -c $<


main-1: \
	$(OBJECTS_DIRS)/collections-fifo.o \
	$(OBJECTS_DIRS)/collections-slice.o \
	$(OBJECTS_DIRS)/db-db.o \
	$(OBJECTS_DIRS)/mpegts-mpegts.o \
	$(OBJECTS_DIRS)/mpegts-header.o \
	$(OBJECTS_DIRS)/mpegts-adaption.o \
	$(OBJECTS_DIRS)/mpegts-psi.o \
	$(OBJECTS_DIRS)/mpegts-psi-cat.o \
	$(OBJECTS_DIRS)/mpegts-psi-nit.o \
	$(OBJECTS_DIRS)/mpegts-psi-pat.o \
	$(OBJECTS_DIRS)/mpegts-psi-pmt.o \
	$(OBJECTS_DIRS)/mpegts-psi-sdt.o \
	$(OBJECTS_DIRS)/mpegts-pes.o \
	$(OBJECTS_DIRS)/mpegts-es.o \
	$(OBJECTS_DIRS)/h264-h264.o \
	$(OBJECTS_DIRS)/h264-nal-sps.o \
	$(OBJECTS_DIRS)/h264-nal-pps.o \
	$(OBJECTS_DIRS)/h264-nal-aud.o \
	$(OBJECTS_DIRS)/h264-nal-sei.o \
	$(OBJECTS_DIRS)/h264-nal-slice-idr.o \
	$(OBJECTS_DIRS)/h264-bitstream.o \
	$(OBJECTS_DIRS)/url-url.o \
	$(OBJECTS_DIRS)/io-udp.o \
	$(OBJECTS_DIRS)/io-file.o \
	$(OBJECTS_DIRS)/io-io.o \
	$(OBJECTS_DIRS)/config.o \
	$(OBJECTS_DIRS)/main-1.o
	$(CC) $(CCFLAGS) -o $(OBJECTS_DIRS)/$@ $+ $(LDFLAGS)

main-2: \
	$(OBJECTS_DIRS)/collections-fifo.o \
	$(OBJECTS_DIRS)/collections-slice.o \
	$(OBJECTS_DIRS)/db-db.o \
	$(OBJECTS_DIRS)/mpegts-mpegts.o \
	$(OBJECTS_DIRS)/mpegts-header.o \
	$(OBJECTS_DIRS)/mpegts-adaption.o \
	$(OBJECTS_DIRS)/mpegts-psi.o \
	$(OBJECTS_DIRS)/mpegts-psi-cat.o \
	$(OBJECTS_DIRS)/mpegts-psi-nit.o \
	$(OBJECTS_DIRS)/mpegts-psi-pat.o \
	$(OBJECTS_DIRS)/mpegts-psi-pmt.o \
	$(OBJECTS_DIRS)/mpegts-psi-sdt.o \
	$(OBJECTS_DIRS)/mpegts-pes.o \
	$(OBJECTS_DIRS)/mpegts-es.o \
	$(OBJECTS_DIRS)/h264-h264.o \
	$(OBJECTS_DIRS)/h264-nal-sps.o \
	$(OBJECTS_DIRS)/h264-nal-pps.o \
	$(OBJECTS_DIRS)/h264-nal-aud.o \
	$(OBJECTS_DIRS)/h264-nal-sei.o \
	$(OBJECTS_DIRS)/h264-nal-slice-idr.o \
	$(OBJECTS_DIRS)/h264-bitstream.o \
	$(OBJECTS_DIRS)/url-url.o \
	$(OBJECTS_DIRS)/io-udp.o \
	$(OBJECTS_DIRS)/io-file.o \
	$(OBJECTS_DIRS)/io-io.o \
	$(OBJECTS_DIRS)/va-parser.o \
	$(OBJECTS_DIRS)/config.o \
	$(OBJECTS_DIRS)/main-2.o
	$(CC) $(CCFLAGS) -o $(OBJECTS_DIRS)/$@ $+ $(LDFLAGS)

collections-slice-test: \
	$(OBJECTS_DIRS)/collections-slice.o \
	$(OBJECTS_DIRS)/collections-slice-test.o
	$(CC) $(CCFLAGS) -o $(OBJECTS_DIRS)/$@ $+ $(LDFLAGS)

bin-dir:
	mkdir -p $(OBJECTS_DIRS)

build: bin-dir main-1 main-2 collections-slice-test

clean:
	rm -rvf $(OBJECTS_DIRS)/*.o
	rm -rvf $(OBJECTS_DIRS)/main

# tsplay ../tmp/hd.ts 239.1.1.1:5500 -loop
run:
	# $(OBJECTS_DIRS)/main 2>&1 >out.log
	$(OBJECTS_DIRS)/main -i 239.1.1.1:5500
