SHELL := /bin/bash

CC:= gcc
CXX:= g++
CCFLAGS := -O0 -ggdb -g -g1

PATH_X265 := $(shell realpath ../../../x265)

INCLUDEFLAS := -I${PATH_X265}/source
INCLUDEFLAS += -I${PATH_X265}/build/linux

LDFLAGS := -L${PATH_X265}/build/linux
LDFLAGS += -Wl,-Bstatic
LDFLAGS += -lx265
LDFLAGS += -Wl,-Bdynamic
LDFLAGS += -ldl
LDFLAGS += -lpthread

OBJECTS_DIRS := ./bin


$(OBJECTS_DIRS)/enc.o: ./enc.c
	$(CC) $(CCFLAGS) $(INCLUDEFLAS) -o $@ -c $<

$(OBJECTS_DIRS)/encs.o: ./encs.c
	$(CC) $(CCFLAGS) $(INCLUDEFLAS) -o $@ -c $<

$(OBJECTS_DIRS)/chan.o: ./chan.c
	$(CC) $(CCFLAGS) $(INCLUDEFLAS) -o $@ -c $<

$(OBJECTS_DIRS)/profile.o: ./profile.c
	$(CC) $(CCFLAGS) $(INCLUDEFLAS) -o $@ -c $<

$(OBJECTS_DIRS)/buf.o: ./buf.c
	$(CC) $(CCFLAGS) $(INCLUDEFLAS) -o $@ -c $<

$(OBJECTS_DIRS)/bufs.o: ./bufs.c
	$(CC) $(CCFLAGS) $(INCLUDEFLAS) -o $@ -c $<

$(OBJECTS_DIRS)/msg-i.o: ./msg-i.c
	$(CC) $(CCFLAGS) $(INCLUDEFLAS) -o $@ -c $<

$(OBJECTS_DIRS)/msg-o.o: ./msg-o.c
	$(CC) $(CCFLAGS) $(INCLUDEFLAS) -o $@ -c $<

$(OBJECTS_DIRS)/signal.o: ./signal.c
	$(CC) $(CCFLAGS) $(INCLUDEFLAS) -o $@ -c $<

$(OBJECTS_DIRS)/main.o: ./main.c
	$(CC) $(CCFLAGS) $(INCLUDEFLAS) -o $@ -c $<


main: \
	$(OBJECTS_DIRS)/encs.o    \
	$(OBJECTS_DIRS)/enc.o     \
	$(OBJECTS_DIRS)/chan.o    \
	$(OBJECTS_DIRS)/profile.o \
	$(OBJECTS_DIRS)/buf.o     \
	$(OBJECTS_DIRS)/bufs.o    \
	$(OBJECTS_DIRS)/msg-i.o   \
	$(OBJECTS_DIRS)/msg-o.o   \
	$(OBJECTS_DIRS)/signal.o  \
	$(OBJECTS_DIRS)/main.o
	$(CXX) $(CCFLAGS) -o $(OBJECTS_DIRS)/$@ $+ $(LDFLAGS)


run:
	# ./bin/main </tmp/egorov/1920x1080-yuv420.yuv
	./bin/main </tmp/egorov/1920x1080-yuv420-small.yuv


bin-dir:
	mkdir -p $(OBJECTS_DIRS)

build: bin-dir main

clean:
	rm -rvf $(OBJECTS_DIRS)/*.o
	rm -rvf $(OBJECTS_DIRS)/main
