CXX := g++
CXXFLAGS := -g

LDFLAGS += -L/usr/lib64 -ldl -lstdc++

OBJECTS_DIRS := ./bin


$(OBJECTS_DIRS)/main.o: ./main.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

main: $(OBJECTS_DIRS)/main.o
	$(CC) $(CCFLAGS) -o $(OBJECTS_DIRS)/$@ $+ $(LDFLAGS)

bin-dir:
	mkdir -p $(OBJECTS_DIRS)

build: bin-dir main

run:
	$(OBJECTS_DIRS)/main
