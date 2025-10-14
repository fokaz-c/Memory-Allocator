#ifndef MEMALLOC_INTERNAL_H
#define MEMALLOC_INTERNAL_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Memory block metadata structure.
 * Stored immediately before each allocated/free block.
 */
typedef struct mem_block {
    size_t size;
    bool is_Free;
    struct mem_block *prev;
    struct mem_block *next;
} mem_block;

/**
 * Initialize a memory block with given size.
 * Sets is_Free to true, prev and next to NULL.
 */
void mem_block_init(mem_block *block, size_t size);

/**
 * Remove a block from the free list and mark as allocated.
 */
void remove_from_free_mem_list(mem_block *block);

/**
 * Add a block to the free list and coalesce with neighbors.
 */
void add_to_free_mem_block_list(mem_block *block);

/**
 * Find the best-fit free block for given size.
 * @return Pointer to best-fit block, or NULL if none found
 */
mem_block *find_free_block(size_t size);

/**
 * Split a large block into allocated portion and smaller free block.
 */
void split_block(mem_block *block, size_t size);

#endif /* MEMALLOC_INTERNAL_H */
