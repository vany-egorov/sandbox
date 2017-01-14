CXX := g++
CXXFLAGS := -g

CC:= gcc
CCFLAGS := -g -pthread -lrt -fPIC

LDFLAGS += -L/usr/lib64 -ldl -lstdc++

BIN_DIR := ./bin
OBJS_DIR := ./obj

$(OBJS_DIR)/main.o: ./main.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/chan.o: ./chan.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/msg-in.o: ./msg-in.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/msg-out.o: ./msg-out.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/msgs.o: ./msgs.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/enc.o: ./enc.c
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJS_DIR)/encs.o: ./encs.c
	$(CC) $(CCFLAGS) -o $@ -c $<

main: \
	$(OBJS_DIR)/chan.o \
	$(OBJS_DIR)/msg-in.o \
	$(OBJS_DIR)/msg-out.o \
	$(OBJS_DIR)/msgs.o \
	$(OBJS_DIR)/enc.o \
	$(OBJS_DIR)/encs.o \
	$(OBJS_DIR)/main.o
	$(CC) $(CCFLAGS) -o $(BIN_DIR)/$@ $+ $(LDFLAGS)

ensure-dirs:
	mkdir -p $(OBJS_DIR)
	mkdir -p $(BIN_DIR)

build: ensure-dirs main

clean:
	rm -rvf $(BIN_DIR)/*.o
	rm -rvf $(BIN_DIR)/*.so
	rm -rvf $(BIN_DIR)/*.a
	rm -rvf $(BIN_DIR)/main

	rm -rvf $(OBJS_DIR)/*.o
	rm -rvf $(OBJS_DIR)/*.so
	rm -rvf $(OBJS_DIR)/*.a
	rm -rvf $(OBJS_DIR)/main
