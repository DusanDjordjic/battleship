CWD=$(shell pwd)

CC=gcc
CFLAGS=-Wall -Wextra -I$(CWD)/include -Wpedantic
LFLAGS=-pthread -L$(CWD)/external/criterion-2.4.2 -lcriterion -Wl,-rpath,$(CWD)/external/criterion-2.4.2

# Folders
SRC=src
OBJ=obj
BIN=bin
TESTS=tests

SERVER_SRCS=$(SRC)/start_server.c
SERVER_SRCS_BINARY=$(SRC)/bin/server.c
SERVER_OBJS=$(patsubst %.c, $(OBJ)/%.o,$(notdir $(SERVER_SRCS)))
SERVER_OBJS_BINARY=$(patsubst %.c, $(OBJ)/%.o,$(notdir $(SERVER_SRCS_BINARY)))
SERVER_BIN=$(BIN)/server.out

CLIENT_SRCS=
CLIENT_SRCS_BINARY=$(SRC)/bin/client.c
CLIENT_OBJS=$(patsubst %.c, $(OBJ)/%.o,$(notdir $(CLIENT_SRCS)))
CLIENT_OBJS_BINARY=$(patsubst %.c, $(OBJ)/%.o,$(notdir $(CLIENT_SRCS_BINARY)))
CLIENT_BIN=$(BIN)/client.out

TESTS_SRCS=$(TESTS)/test_tcp_server.c
TESTS_OBJS=$(patsubst %.c, $(OBJ)/%.o,$(notdir $(TESTS_SRCS)))
TESTS_ALL_SRCS=$(SERVER_SRCS) $(CLIENT_SRCS) $(TESTS_SRCS)
TESTS_ALL_OBJS=$(SERVER_OBJS) $(CLIENT_OBJS) $(TESTS_OBJS)
TEST_BIN=$(BIN)/test_runner.out

.PHONY: all
all: server client

.PHONY: server
server: $(SERVER_BIN)

.PHONY: client
client: $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_OBJS) $(SERVER_OBJS_BINARY)
	$(CC) -o $@ $^ $(LFLAGS)

$(CLIENT_BIN): $(CLIENT_OBJS) $(CLIENT_OBJS_BINARY)
	$(CC) -o $@ $^ $(LFLAGS)

$(OBJ)/%.o: $(SRC)/bin/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJ)/%.o: $(TESTS)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY:test
test: $(TEST_BIN)
	./$<

$(TEST_BIN): $(TESTS_ALL_OBJS)
	$(CC) -o $@ $^ $(LFLAGS)

# Used to create object and binary folders

.PHONY: setup
setup: $(OBJ) $(BIN)

$(OBJ):
	mkdir -p $@

$(BIN):
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf $(SERVER_BIN) $(SERVER_OBJS) $(SERVER_OBJS_BINARY)\
	       $(CLIENT_BIN) $(CLIENT_OBJS) $(CLIENT_OBJS_BINARY)\
		   $(TESTS_ALL_OBJS) $(TEST_BIN)
