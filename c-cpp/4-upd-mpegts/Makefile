CXX := g++
CXXFLAGS := -g

CC:= gcc
# CCFLAGS :=-g -std=c11 -lpthread -lrt
CCFLAGS :=-g -lpthread -lrt

LDFLAGS += -L/usr/lib64 -ldl -lstdc++

OBJECTS_DIRS := ./bin


$(OBJECTS_DIRS)/main.o: ./main.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/fifo.o: ./fifo.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/config.o: ./config.c
	$(CC) $(CCFLAGS) -o $@ -c $<

main: $(OBJECTS_DIRS)/main.o $(OBJECTS_DIRS)/fifo.o $(OBJECTS_DIRS)/config.o
	$(CC) $(CCFLAGS) -o $(OBJECTS_DIRS)/$@ $+ $(LDFLAGS)

bin-dir:
	mkdir -p $(OBJECTS_DIRS)

build: bin-dir main

run:
	# $(OBJECTS_DIRS)/main 2>&1 >out.log
	$(OBJECTS_DIRS)/main
