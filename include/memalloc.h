#ifndef MEMALLOC_H
#define MEMALLOC_H

#include <stddef.h>

/**
 * Allocate memory of specified size.
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
void *ma_malloc(size_t size);

/**
 * Free previously allocated memory.
 * @param ptr Pointer to memory to free (NULL is safe)
 */
void ma_free(void *ptr);

/**
 * Allocate and zero-initialize memory for an array.
 * @param n Number of elements
 * @param size Size of each element
 * @return Pointer to allocated memory, or NULL on failure
 */
void *ma_calloc(size_t n, size_t size);

/**
 * Resize previously allocated memory.
 * @param ptr Pointer to memory to resize (NULL acts like malloc)
 * @param size New size in bytes (0 acts like free)
 * @return Pointer to resized memory, or NULL on failure
 */
void *ma_realloc(void *ptr, size_t size);

/**
 * Print the current state of the free list.
 * Useful for debugging and understanding allocator behavior.
 */
//uncomment to use this
//void ma_print_free_list(void);

#endif /* MEMALLOC_H */
