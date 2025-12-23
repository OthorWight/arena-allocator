# Arena Allocator

A robust, single-file C memory arena allocator designed for high-performance projects. It features **O(1) memory complexity** for repetitive tasks via a self-cleaning block mechanism.

## Features

* **Single-Header Library:** Just drop `include/arena.h` into your project.
* **Zero-Overhead Reset:** `arena_reset()` invalidates memory instantly without OS calls.
* **Self-Cleaning:** Automatically detects and frees fragmentation when data sizes grow (e.g., infinite loops).
* **Scratchpad Support:** Scoped temporary memory for functions.
* **Python Integration:** Compile as a shared library for high-speed C extensions.

## Directory Structure

```text
.
├── include/        # The library header (arena.h)
├── src/            # Source for Python shared library
├── examples/       # Usage examples (Basic, Threaded, Fibonacci)
├── build/          # Compiled executables (created by make)
└── Makefile        # Build automation
```

## Quick Start

### 1. Build Everything
Run the makefile to compile all examples and the shared library:

```bash
make
```

### 2. Run Examples

**Stress Test (Fibonacci):**
Calculates `Fib(5,000,000)` (over 1 million digits) using constant memory (~93KB) and a custom Base 1e9 BigInt implementation.
```bash
./build/fib_pingpong
```

**Thread Safety:**
Demonstrates creating isolated arenas for separate threads (lock-free).
```bash
./build/threaded_test
```

## Integration Guide

To use this in your own project, just copy `include/arena.h`.

In **one** source file, define the implementation:

```c
#define ARENA_IMPLEMENTATION
#include "arena.h"
```

In other files, just include it:

```c
#include "arena.h"
```

### Example Usage

```c
Arena a = {0};
arena_init(&a);

// Allocate simple arrays
int *nums = arena_alloc_array(&a, int, 1000);

// Allocate structs
MyStruct *obj = arena_alloc_struct(&a, MyStruct);

// Reset (Reuse memory without freeing to OS)
arena_reset(&a);

// Final cleanup
arena_free(&a);
```

## License

Public Domain / MIT.
