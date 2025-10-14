CC = clang
CFLAGS = -Wall -Wextra -std=c11 -D_DEFAULT_SOURCE -Iinclude -pthread
CFLAGS_DEBUG = $(CFLAGS) -g

BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)/lib
BIN_DIR = $(BUILD_DIR)/bin

LIB_TARGET = $(LIB_DIR)/libmemalloc.a
EXAMPLE_TARGET = $(BIN_DIR)/benchmark

LIB_SRCS = src/memalloc.c
LIB_OBJS = $(LIB_SRCS:src/%.c=$(BUILD_DIR)/%.o)

EXAMPLE_SRCS = examples/benchmark.c
EXAMPLE_OBJS = $(EXAMPLE_SRCS:examples/%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR) $(LIB_DIR) $(LIB_TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(LIB_TARGET): $(LIB_OBJS)
	ar rcs $(LIB_TARGET) $(LIB_OBJS)

$(EXAMPLE_TARGET): $(EXAMPLE_OBJS) $(LIB_TARGET) $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(EXAMPLE_TARGET) $(EXAMPLE_OBJS) $(LIB_TARGET)

$(BUILD_DIR)/memalloc.o: src/memalloc.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/benchmark.o: examples/benchmark.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: $(BUILD_DIR) $(LIB_DIR) $(BIN_DIR)
	$(CC) $(CFLAGS_DEBUG) -c $(LIB_SRCS) -o $(BUILD_DIR)/memalloc.o
	ar rcs $(LIB_TARGET) $(BUILD_DIR)/memalloc.o
	$(CC) $(CFLAGS_DEBUG) -c $(EXAMPLE_SRCS) -o $(BUILD_DIR)/benchmark.o
	$(CC) $(CFLAGS_DEBUG) -o $(EXAMPLE_TARGET) $(BUILD_DIR)/benchmark.o $(LIB_TARGET)
	gdb ./$(EXAMPLE_TARGET)

clean:
	rm -rf $(BUILD_DIR)

run: $(EXAMPLE_TARGET)
	./$(EXAMPLE_TARGET)

.PHONY: all clean run debug
