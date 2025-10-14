# Memalloc – Memory Allocator

A high-performance, thread-safe dynamic memory allocator implementation from scratch. Memalloc replicates the core functionality of standard C library functions (`malloc`, `calloc`, `realloc`, `free`) with custom heap management and intelligent fragmentation control.

## Features

**Core Functionality**
- Drop-in replacement for standard C memory functions: `ma_malloc()`, `ma_free()`, `ma_calloc()`, and `ma_realloc()`
- Direct heap management using `mmap()` for efficient memory provisioning
- 1MB pre-allocated heap pool with dynamic block management
- Zero external dependencies

**Memory Management Strategies**
- **Best-fit allocation** – Selects the smallest available block that satisfies the request, minimizing internal fragmentation
- **Block splitting** – Divides oversized blocks into allocated and free portions to reduce wasted space
- **Automatic coalescing** – Merges adjacent free blocks to mitigate external fragmentation
- **Doubly-linked free list** – Efficient tracking of freed blocks with straightforward O(n) search

**Thread Safety**
- Recursive mutex protection for all public functions
- Thread-safe heap initialization and concurrent allocation/deallocation
- Safe for use in multi-threaded applications
- Automatic cleanup on program exit

## Architecture

Each memory block is prefixed with a `mem_block` metadata header:

```c
typedef struct mem_block {
    size_t size;              // User-requested allocation size
    bool is_Free;             // Current allocation status
    struct mem_block *prev;   // Link to previous free block
    struct mem_block *next;   // Link to next free block
} mem_block;
```

The allocator maintains a free list of deallocated blocks. When `ma_malloc()` is called, it searches the free list using a best-fit strategy. If no suitable block exists, it expands the heap pool. When `ma_free()` is called, the block is returned to the free list and automatically coalesced with adjacent free blocks to reduce fragmentation.

## API Reference

**`void *ma_malloc(size_t size)`**
Allocates `size` bytes of uninitialized heap memory. Returns a pointer to the allocated memory or `NULL` on failure.

**`void *ma_calloc(size_t n, size_t size)`**
Allocates memory for an array of `n` elements, each `size` bytes. All bytes are initialized to zero. Returns a pointer or `NULL` on failure.

**`void *ma_realloc(void *ptr, size_t size)`**
Resizes an existing allocation to `size` bytes. If the block can be expanded in-place using adjacent free space, the original pointer is returned. Otherwise, a new block is allocated, contents are copied, and the old block is freed.

**`void ma_free(void *ptr)`**
Deallocates memory previously allocated by `ma_malloc()`, `ma_calloc()`, or `ma_realloc()`. The block is added to the free list and automatically coalesced with neighbors.

**`void ma_print_free_list(void)`**
Prints the current state of the free list and heap usage statistics. Useful for diagnosing fragmentation and memory availability.

## Building

**Build the static library:**
```bash
make
```

**Run benchmarks and examples:**
```bash
make run
```

**Debug with GDB:**
```bash
make debug
```

**Clean build artifacts:**
```bash
make clean
```

## Performance Analysis

- **Allocation Time** – O(n) where n is the number of free blocks (best-fit search)
- **Deallocation Time** – O(n) due to coalescing and free list updates
- **Memory Overhead** – Each block carries a `mem_block` header (~32 bytes on 64-bit systems)
- **Fragmentation** – Minimized through block splitting and coalescing, though external fragmentation may accumulate with certain allocation patterns

For production systems requiring extreme performance, consider upgrading to advanced allocation strategies such as segregated fits, buddy allocation, or balanced search trees for O(log n) lookups.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.
