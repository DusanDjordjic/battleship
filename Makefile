CWD=$(shell pwd)

CC=gcc
CFLAGS=-Wall -Wextra -I$(CWD) -Wpedantic
# LFLAGS=-pthread -lncurses
LFLAGS=-pthread -lvector -L$(CWD)/external/vector -Wl,-rpath,'$$ORIGIN'/../external/vector 
TEST_LFLAGS=-L$(CWD)/external/criterion-2.4.2 -I$(CWD) -lcriterion -Wl,-rpath,'$$ORIGIN'/../external/criterion-2.4.2

# Folders
SRC=src
OBJ=obj
BIN=bin
INC=include
TESTS=tests

HEADERS=$(INC)/args.h $(INC)/errors.h $(INC)/io.h \
	$(INC)/menu.h $(INC)/server.h $(INC)/state.h $(INC)/server.h \
	$(INC)/globals.h $(INC)/messages.h $(INC)/server_handlers.h	\
	$(INC)/server_utils.h $(INC)/game.h $(INC)/vector/vector.h \
	$(INC)/game_ship.h $(INC)/game_results.h

SERVER_SRCS=$(SRC)/error.c $(SRC)/server_start.c $(SRC)/args.c $(SRC)/messages.c \
			$(SRC)/users.c $(SRC)/server_handlers.c $(SRC)/server_utils.c $(SRC)/game.c $(SRC)/game_results.c
SERVER_SRCS_BINARY=$(SRC)/bin/server.c
SERVER_OBJS=$(patsubst %.c, $(OBJ)/%.o,$(notdir $(SERVER_SRCS)))
SERVER_OBJS_BINARY=$(patsubst %.c, $(OBJ)/%.o,$(notdir $(SERVER_SRCS_BINARY)))
SERVER_BIN=$(BIN)/server.out

CLIENT_SRCS=$(SRC)/io.c $(SRC)/error.c $(SRC)/menu.c $(SRC)/args.c $(SRC)/messages.c $(SRC)/game_ship.c \
			$(SRC)/coordinate.c
CLIENT_SRCS_BINARY=$(SRC)/bin/client.c
CLIENT_OBJS=$(patsubst %.c, $(OBJ)/%.o,$(notdir $(CLIENT_SRCS)))
CLIENT_OBJS_BINARY=$(patsubst %.c, $(OBJ)/%.o,$(notdir $(CLIENT_SRCS_BINARY)))
CLIENT_BIN=$(BIN)/client.out

TESTS_SRCS=$(TESTS)/test_tcp_server.c $(TESTS)/test_coordinate.c $(TESTS)/test_game_ship.c
TESTS_OBJS=$(patsubst %.c, $(OBJ)/%.o,$(notdir $(TESTS_SRCS)))
TESTS_ALL_SRCS=$(SERVER_SRCS) $(CLIENT_SRCS) $(TESTS_SRCS)
TESTS_ALL_OBJS=$(SERVER_OBJS) $(CLIENT_OBJS) $(TESTS_OBJS)
TEST_BIN=$(BIN)/test_runner.out

.PHONY: build
build: server client

.PHONY: server
server: $(SERVER_BIN)

.PHONY: client
client: $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_OBJS) $(SERVER_OBJS_BINARY)
	$(CC) -o $@ $^ $(LFLAGS)

$(CLIENT_BIN): $(CLIENT_OBJS) $(CLIENT_OBJS_BINARY)
	$(CC) -o $@ $^ $(LFLAGS)

$(OBJ)/%.o: $(SRC)/bin/%.c $(HEADERS)
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJ)/%.o: $(TESTS)/%.c $(HEADERS)
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJ)/%.o: $(SRC)/%.c $(HEADERS)
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: test
test: $(TEST_BIN)
	./$<

$(TEST_BIN): $(TESTS_ALL_OBJS)
	$(CC) -o $@ $^ $(LFLAGS) $(TEST_LFLAGS)

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
