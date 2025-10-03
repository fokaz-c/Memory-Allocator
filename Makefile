CC = clang
CFLAGS = -Wall -Wextra -std=c11 -D_DEFAULT_SOURCE -Iinclude
CFLAGS_DEBUG = $(CFLAGS) -g           # debug symbols
TARGET = build/memalloc
BUILD_DIR = build

SRCS = src/main.c src/memalloc.c
OBJS = $(SRCS:src/%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Object files
$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Debug build + run in gdb
debug: $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) $(SRCS) -o $(TARGET)
	gdb ./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run debug
