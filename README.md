# Custom Memory Allocator in C

This project is a **custom implementation of a dynamic memory allocator in C**, built from scratch to explore how low-level heap management works.
It replicates the behavior of standard C library functions like `malloc`, `calloc`, `realloc`, and `free` using the `sbrk()` system call for direct heap manipulation.

---

## Overview

The allocator demonstrates how dynamic memory allocation can be managed manually in user space.
It implements core allocation strategies and data structures used in real-world memory managers, with a focus on **efficiency**, **fragmentation control**, and **extensibility** for future improvements.

---

## Implementation Details

The allocator is built around these core design elements:

* **Heap Management** – Uses `sbrk()` to expand the program’s data segment on demand.
* **Data Structure** – A **doubly linked list** maintains free and allocated blocks. Each block starts with a `mem_block` header containing metadata such as block size, free status, and links to adjacent blocks.
* **Allocation Strategy** – Employs a **best-fit** approach to minimize internal fragmentation by selecting the smallest available block that fits the request.
* **Block Splitting** – If a free block is significantly larger than required, it is split into an allocated and a free portion.
* **Coalescing** – Adjacent free blocks are merged automatically by `coalesce_with_neighbors()` to reduce external fragmentation.

---

## API

The allocator provides the following functions:

* `void *ma_malloc(size_t size)` – Allocates `size` bytes of uninitialized memory.
* `void ma_free(void *ptr)` – Deallocates memory and returns it to the free list.
* `void *ma_calloc(size_t n, size_t size)` – Allocates and zero-initializes memory for an array.
* `void *ma_realloc(void *ptr, size_t size)` – Resizes an existing allocation, preserving its contents.

---

## Build and Run

A `Makefile` is provided for easy compilation and testing.

**Build the project:**

```bash
make
```

**Run the executable:**

```bash
make run
```

**Clean build artifacts:**

```bash
make clean
```

---

## Ongoing Development

Active development is focused on **enhancing performance, safety, and usability**.
Current efforts include:

* **Thread Safety** – Introducing synchronization primitives to make the allocator multi-threaded.
* **Performance Profiling** – Adding benchmarking tools to measure allocation and free times.
* **Debug Utilities** – Building diagnostic APIs for leak detection and heap visualization.
---

## References and Further Reading

For more context on the concepts behind this project:

* [Dynamic Memory Allocation in C](./docs/dynamic-memory-allocation-in-c.md)
* [Memory Layout in C](./docs/memory-layout-c.md)
