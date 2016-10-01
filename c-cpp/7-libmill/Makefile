CXX := g++
CXXFLAGS := -g

CC:= gcc
# CCFLAGS :=-g -std=c11 -lpthread -lrt
CCFLAGS :=-g -lpthread -lrt

LDFLAGS := -L/usr/lib64 -ldl -lstdc++
LDFLAGS += -L./libmill/.libs -lmill

OBJECTS_DIRS := ./bin


$(OBJECTS_DIRS)/main.o: ./main.c
	$(CC) $(CCFLAGS) -o $@ -c $<

main: $(OBJECTS_DIRS)/main.o
	$(CC) $(CCFLAGS) -o $(OBJECTS_DIRS)/$@ $+ $(LDFLAGS)

bin-dir:
	mkdir -p $(OBJECTS_DIRS)

build: bin-dir main

run:
	export LD_LIBRARY_PATH="./libmill/.libs:$${LD_LIBRARY_PATH}"; \
	$(OBJECTS_DIRS)/main

libmill-get:
	rm -rvf ./libmill && \
	git clone https://github.com/sustrik/libmill.git && \
	cd ./libmill && \
		./autogen.sh && \
		./configure && \
		make && \
		make check
