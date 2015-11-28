CXX := g++
CXXFLAGS := -g

CC:= gcc
# CCFLAGS :=-g -std=c11 -lpthread -lrt
CCFLAGS :=-g -lpthread -lrt

LDFLAGS += -L/usr/lib64 -ldl -lstdc++

OBJECTS_DIRS := ./bin


$(OBJECTS_DIRS)/main.o: ./main.c
	$(CC) $(CCFLAGS) -o $@ -c $<

main: $(OBJECTS_DIRS)/main.o
	$(CC) $(CCFLAGS) -o $(OBJECTS_DIRS)/$@ $+ $(LDFLAGS)

bin-dir:
	mkdir -p $(OBJECTS_DIRS)

build: bin-dir main

run:
	$(OBJECTS_DIRS)/main
