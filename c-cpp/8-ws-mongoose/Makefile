CXX := g++
CXXFLAGS := -g

CC:= gcc
# CCFLAGS :=-g -std=c11 -lpthread -lrt
CCFLAGS :=-g -lpthread -lrt

LDFLAGS := -L/usr/lib64 -ldl -lstdc++

OBJECTS_DIRS := ./bin

$(OBJECTS_DIRS)/mongoose.o: ./mongoose/mongoose.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJECTS_DIRS)/main.o: ./main.c
	$(CC) $(CCFLAGS) -o $@ -c $<

main: $(OBJECTS_DIRS)/main.o $(OBJECTS_DIRS)/mongoose.o
	$(CC) $(CCFLAGS) -o $(OBJECTS_DIRS)/$@ $+ $(LDFLAGS)

bin-dir:
	mkdir -p $(OBJECTS_DIRS)

build: bin-dir main

mongoose-get:
	rm -rvf ./mongoose
	rm -rvf ./mongoose-6.5
	rm -rvf ./mongoose.tar.gz
	curl -XGET -L "https://github.com/cesanta/mongoose/archive/6.5.tar.gz" > ./mongoose.tar.gz
	tar -xvf ./mongoose.tar.gz
	mv ./mongoose-6.5 ./mongoose
	rm -rvf ./mongoose.tar.gz
