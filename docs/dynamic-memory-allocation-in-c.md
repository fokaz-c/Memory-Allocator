# Dynamic Memory Allocation in C

## Overview

Dynamic Memory Allocation gives programmers control over memory allocation and deallocation:

  - Allocate memory at runtime to handle data of varying sizes
  - Use heap memory instead of stack
  - Resize memory as needed

> "With great power comes great responsibility." — Uncle Ben, *Spider-Man*

-----

## Disadvantages

Dynamic Memory Allocation comes with several disadvantages:

  - **Manual memory management** - You must explicitly free what you allocate
  - **Memory leaks** - Forgotten `free()` calls lead to gradual memory exhaustion
  - **Dangling pointers** - Using freed memory causes undefined behavior
  - **Fragmentation** - Repeated allocation/deallocation can fragment heap memory
  - **Slower than stack** - Heap allocation is significantly slower than stack allocation
  - **No bounds checking** - Easy to write beyond allocated memory (buffer overflows)
  - **Debugging difficulty** - Memory errors often manifest far from their source

-----

## Standard Library

All functions are available in `<stdlib.h>`.

-----

## Functions

### 1\. `malloc()`

**Memory Allocation** - Allocates a single block of contiguous memory.

**Syntax:**

```c
void* malloc(size_t size);
```

**Key Points:**

  - Returns a pointer to the allocated memory, or `NULL` on failure
  - Memory is uninitialized (contains garbage values)
  - Must cast the return value to the appropriate type

**Usage:**

```c
int *ptr = (int*)malloc(5 * sizeof(int));
if (ptr == NULL) {
    // Handle allocation failure
}
// Use ptr...
free(ptr);
```

-----

### 2\. `calloc()`

**Contiguous Allocation** - Allocates memory for an array of elements.

**Syntax:**

```c
void* calloc(size_t num, size_t size);
```

**Key Points:**

  - Allocates memory for `num` elements of `size` bytes each
  - Initializes all bytes to zero
  - Returns `NULL` on failure

**Usage:**

```c
int *arr = (int*)calloc(5, sizeof(int));
if (arr == NULL) {
    // Handle allocation failure
}
// All elements initialized to 0
free(arr);
```

-----

### 3\. `realloc()`

**Reallocation** - Resizes a previously allocated memory block.

**Syntax:**

```c
void* realloc(void* ptr, size_t new_size);
```

**Key Points:**

  - Can expand or shrink an existing memory block
  - Preserves original data up to the minimum of the old and new size
  - May move the block to a new location
  - Returns `NULL` on failure (the original block remains valid)
  - If `ptr` is `NULL`, it behaves like `malloc()`
  - If `new_size` is 0, it behaves like `free()`

**Usage:**

```c
int *ptr = (int*)malloc(5 * sizeof(int));
// Need more space...
int *temp = (int*)realloc(ptr, 10 * sizeof(int));
if (temp == NULL) {
    // Handle failure, ptr is still valid
} else {
    ptr = temp;
}
free(ptr);
```

-----

### 4\. `free()`

**Deallocation** - Releases dynamically allocated memory.

**Syntax:**

```c
void free(void* ptr);
```

**Key Points:**

  - Deallocates memory previously allocated by `malloc()`, `calloc()`, or `realloc()`
  - Does nothing if `ptr` is `NULL`
  - After freeing, the pointer becomes dangling (good practice to set it to `NULL`)
  - Double-freeing causes undefined behavior

**Usage:**

```c
int *ptr = (int*)malloc(sizeof(int));
// Use ptr...
free(ptr);
ptr = NULL;  // Prevent dangling pointer
```

-----

## Comparison Table

| Function | Initialization | Parameters | Use Case |
|----------|---------------|------------|----------|
| `malloc()` | Uninitialized | Size in bytes | General allocation |
| `calloc()` | Zero-initialized | Count + size | Arrays needing zero values |
| `realloc()` | Preserves data | Pointer + new size | Resizing existing allocation |
| `free()` | N/A | Pointer | Cleanup |

-----

## Best Practices

1.  **Always check return values** - Handle `NULL` returns gracefully.
2.  **Free allocated memory** - Prevent memory leaks by freeing what you allocate.
3.  **Avoid dangling pointers** - Set pointers to `NULL` after freeing.
4.  **Never double-free** - Free each allocation exactly once.
5.  **Match allocations** - Use `free()` for every successful allocation.
6.  **Use `sizeof`** - Ensures portability and correctness across different platforms.

> "People have to save themselves. One person saving another is impossible." — Oshino Meme, *Monogatari Series*

-----

## Common Mistakes

```c
// Not checking for NULL
int *ptr = (int*)malloc(sizeof(int));
*ptr = 10;  // May crash if allocation failed

// Memory leak
int *ptr = (int*)malloc(sizeof(int));
ptr = NULL;  // Lost reference to allocated memory

// Using freed memory
int *ptr = (int*)malloc(sizeof(int));
free(ptr);
*ptr = 10;  // Undefined behavior

// Double free
free(ptr);
free(ptr);  // Undefined behavior
```

> "If you keep running away, you'll keep losing things that are important to you." — Reina Kousaka, *Sound\! Euphonium*

-----

## Example: Complete Usage

```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    // malloc - uninitialized
    int *arr1 = (int*)malloc(3 * sizeof(int));
    if (arr1 == NULL) return 1;

    // calloc - zero-initialized
    int *arr2 = (int*)calloc(3, sizeof(int));
    if (arr2 == NULL) {
        free(arr1);
        return 1;
    }

    // realloc - resize
    int *temp = (int*)realloc(arr1, 5 * sizeof(int));
    if (temp == NULL) {
        free(arr1);
        free(arr2);
        return 1;
    }
    arr1 = temp;

    // Use arrays...

    // Clean up
    free(arr1);
    free(arr2);

    return 0;
}
```
