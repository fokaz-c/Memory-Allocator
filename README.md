# Custom Memory Allocator in C

This project is a custom implementation of a dynamic memory allocator in C. It demonstrates how a basic memory allocator can be built from the ground up using the `sbrk()` system call to manage the program's heap.

## Implementation Details

The allocator uses the following strategies to manage memory:

*   **Heap Management**: Memory is requested from the operating system using the `sbrk()` system call, which increments the program's data segment.
*   **Data Structure**: A doubly linked list is used to maintain a list of free memory blocks. Each block is preceded by a `mem_block` header that stores metadata, including the block's size, its free status, and pointers to the next and previous blocks in the free list.
*   **Allocation Strategy**: A **best-fit** algorithm is employed. When `ma_malloc` is called, it traverses the free list to find the smallest block that is large enough to satisfy the requested size. This helps to minimize wasted space (internal fragmentation).
*   **Block Splitting**: If a free block is found that is larger than the requested size, it is split. The required portion is returned to the user, and the remainder is kept as a new, smaller free block in the list.
*   **Coalescing**: The allocator includes functionality to merge adjacent free blocks (`coalesce_with_neighbors`). This process helps reduce external fragmentation by creating larger contiguous blocks of free memory.

## API

The following function is currently implemented:

*   `void *ma_malloc(size_t size)`: Allocates `size` bytes of uninitialized memory.

## How to Build and Run

A `Makefile` is provided for easy compilation and execution.

*   **Build the project:**
    ```sh
    make
    ```
    This will compile the source files and place the object files and the final executable in the `build/` directory.

*   **Run the executable:**
    ```sh
    make run
    ```

*   **Clean the build artifacts:**
    ```sh
    make clean
    ```

## Future Work

The current implementation provides a foundation for a more complete memory allocator. Future enhancements will include:

*   **`ma_free(void *ptr)`**: Implement memory deallocation to mark blocks as free and add them back to the free list.
*   **`ma_calloc(size_t n, size_t size)`**: Add support for allocating and zero-initializing memory for an array of elements.
*   **`ma_realloc(void* ptr, size_t size)`**: Implement resizing of existing memory allocations.

## Further Reading

For more background on the concepts used in this project, see the following documents:

*   [Dynamic Memory Allocation in C](./docs/dynamic-memory-allocation-in-c.md)
*   [Memory Layout in C](./docs/memory-layout-c.md)
