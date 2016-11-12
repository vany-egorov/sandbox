CXX := g++
CXXFLAGS := -g

CC:= gcc
# CCFLAGS :=-g -std=c11 -lpthread -lrt
CCFLAGS :=-g -lpthread -lrt

LDFLAGS += -L/usr/lib64 -ldl -lstdc++

OBJECTS_DIRS := ./bin


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

$(OBJECTS_DIRS)/udp.o: ./udp.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/file.o: ./file.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/url.o: ./url.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/io.o: ./io.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/util.o: ./util.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/fifo.o: ./fifo.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/config.o: ./config.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/main.o: ./main.c
	$(CC) $(CCFLAGS) -o $@ -c $<


main: \
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
	$(OBJECTS_DIRS)/fifo.o \
	$(OBJECTS_DIRS)/config.o \
	$(OBJECTS_DIRS)/udp.o \
	$(OBJECTS_DIRS)/file.o \
	$(OBJECTS_DIRS)/url.o \
	$(OBJECTS_DIRS)/io.o \
	$(OBJECTS_DIRS)/main.o
	$(CC) $(CCFLAGS) -o $(OBJECTS_DIRS)/$@ $+ $(LDFLAGS)

bin-dir:
	mkdir -p $(OBJECTS_DIRS)

build: bin-dir main

clean:
	rm -rvf $(OBJECTS_DIRS)/*.o
	rm -rvf $(OBJECTS_DIRS)/main

# tsplay ./tmp/hd.ts 239.1.1.1:5500 -loop
run:
	# $(OBJECTS_DIRS)/main 2>&1 >out.log
	$(OBJECTS_DIRS)/main -i 239.1.1.1:5500
