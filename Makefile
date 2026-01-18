CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude -std=c23
LDFLAGS = 

# Output directory
BUILD_DIR = build

# Targets
TARGETS = $(BUILD_DIR)/basic_test           $(BUILD_DIR)/threaded_test           $(BUILD_DIR)/fib_pingpong           $(BUILD_DIR)/libarena.so

.PHONY: all clean help directories

all: directories $(TARGETS)

directories:
	@mkdir -p $(BUILD_DIR)

# 1. Basic Stress Test
$(BUILD_DIR)/basic_test: examples/basic_test.c include/arena.h
	$(CC) $(CFLAGS) examples/basic_test.c -o $@

# 2. Multi-threaded Example (needs pthread)
$(BUILD_DIR)/threaded_test: examples/threaded_test.c include/arena.h
	$(CC) $(CFLAGS) -pthread examples/threaded_test.c -o $@

# 3. Fibonacci Ping-Pong
$(BUILD_DIR)/fib_pingpong: examples/fib_pingpong.c include/arena.h
	$(CC) $(CFLAGS) examples/fib_pingpong.c -o $@

# 4. Shared Library (needs src/arena_lib.c)
$(BUILD_DIR)/libarena.so: src/arena_lib.c include/arena.h
	$(CC) $(CFLAGS) -fPIC -shared src/arena_lib.c -o $@

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Run 'make' to build everything into the build/ folder."
