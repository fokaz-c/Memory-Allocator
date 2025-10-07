#include "memalloc.h"
#include "memalloc_internal.h"
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * STATIC VARIABLES
 * ============================================================================ */

/* Head of the free block list */
static mem_block *free_mem_block_list_head = NULL;


/* ============================================================================
 * INTERNAL HELPER FUNCTIONS
 * ============================================================================ */

/**
 * Check if two memory blocks are physically adjacent in memory.
 */
static bool are_adjacent(mem_block *first, mem_block *second) {
    void *end_of_first = (char *)first + sizeof(mem_block) + first->size;
    return end_of_first == second;
}

/**
 * Merge a block with neighboring free blocks to reduce fragmentation.
 */
static void coalesce_with_neighbors(mem_block *block) {
    mem_block *current = free_mem_block_list_head;

    while (current != NULL) {
        mem_block *next_iter = current->next;

        if (current == block) {
            current = next_iter;
            continue;
        }

        if (are_adjacent(block, current)) {
            /* Merge: block absorbs current */
            block->size += sizeof(mem_block) + current->size;
            remove_from_free_mem_list(current);
        } else if (are_adjacent(current, block)) {
            /* Merge: current absorbs block */
            current->size += sizeof(mem_block) + block->size;

            /* Remove block from free list */
            if (block->prev != NULL) {
                block->prev->next = block->next;
            } else {
                free_mem_block_list_head = block->next;
            }
            if (block->next != NULL) {
                block->next->prev = block->prev;
            }
            return;
        }

        current = next_iter;
    }
}


/* ============================================================================
 * MEMORY BLOCK MANAGEMENT
 * ============================================================================ */

void mem_block_init(mem_block *block, size_t size) {
    block->size = size;
    block->is_Free = true;
    block->prev = NULL;
    block->next = NULL;
}

void remove_from_free_mem_list(mem_block *block) {
    if (free_mem_block_list_head == NULL || block == NULL) {
        return;
    }

    mem_block *tmp = free_mem_block_list_head;

    while (tmp != NULL) {
        if (tmp == block) {
            if (tmp->prev != NULL) {
                tmp->prev->next = tmp->next;
            } else {
                free_mem_block_list_head = tmp->next;
            }

            if (tmp->next != NULL) {
                tmp->next->prev = tmp->prev;
            }

            block->prev = NULL;
            block->next = NULL;
            block->is_Free = false;
            return;
        }
        tmp = tmp->next;
    }
}

void add_to_free_mem_block_list(mem_block *block) {
    if (block == NULL) {
        return;
    }

    remove_from_free_mem_list(block);

    block->is_Free = true;
    block->prev = NULL;
    block->next = free_mem_block_list_head;

    if (free_mem_block_list_head != NULL) {
        free_mem_block_list_head->prev = block;
    }

    free_mem_block_list_head = block;

    coalesce_with_neighbors(block);
}

mem_block *find_free_block(size_t size) {
    mem_block *current = free_mem_block_list_head;
    mem_block *best_fit = NULL;

    while (current != NULL) {
        if (current->is_Free && current->size >= size) {
            if (best_fit == NULL || current->size < best_fit->size) {
                best_fit = current;

                if (current->size == size) {
                    return best_fit;
                }
            }
        }
        current = current->next;
    }

    return best_fit;
}

void split_block(mem_block *block, size_t size) {
    if (block->size >= size + sizeof(mem_block) + 1) {
        mem_block *new_block = (mem_block *)((char *)block
            + sizeof(mem_block) + size);
        mem_block_init(new_block, block->size - size - sizeof(mem_block));

        block->size = size;

        add_to_free_mem_block_list(new_block);
    }
}


/* ============================================================================
 * PUBLIC API IMPLEMENTATION
 * ============================================================================ */

void *ma_malloc(size_t size) {
    /* Handle edge cases */
    if (size == 0 || size > SIZE_MAX - sizeof(mem_block)) {
        return NULL;
    }

    mem_block *block = find_free_block(size);

    if (block != NULL) {
        remove_from_free_mem_list(block);
        split_block(block, size);
        return (void *)((char *)block + sizeof(mem_block));
    }

    void *ptr = sbrk(sizeof(mem_block) + size);

    if (ptr == (void *)-1) {
        return NULL;
    }

    mem_block *new_block = (mem_block *)ptr;
    mem_block_init(new_block, size);
    new_block->is_Free = false;

    return (void *)((char *)new_block + sizeof(mem_block));
}

void ma_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    mem_block *block = (mem_block *)((char *)ptr - sizeof(mem_block));

    add_to_free_mem_block_list(block);
}

void *ma_calloc(size_t n, size_t size) {
    size_t total_size = n * size;
    if (n == 0 || total_size / n != size) {
        return NULL;
    }

    void *ptr = ma_malloc(total_size);
    if (ptr == NULL) {
        return NULL;
    }

    memset(ptr, 0, total_size);

    return ptr;
}

void *ma_realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return ma_malloc(size);
    }

    if (size == 0) {
        ma_free(ptr);
        return NULL;
    }

    mem_block *current = (mem_block *)((char *)ptr - sizeof(mem_block));
    size_t old_size = current->size;

    if (size <= old_size) {
        return ptr;
    }

    size_t needed = size - old_size;

    mem_block *free_node = free_mem_block_list_head;
    while (free_node != NULL) {
        if (are_adjacent(current, free_node) && free_node->size >= needed) {
            remove_from_free_mem_list(free_node);
            current->size += sizeof(mem_block) + free_node->size;

            if (current->size > size + sizeof(mem_block) + 1) {
                split_block(current, size);
            }

            return ptr;
        }
        free_node = free_node->next;
    }

    void *newptr = ma_malloc(size);
    if (newptr == NULL) {
        return NULL;
    }

    memcpy(newptr, ptr, old_size);

    ma_free(ptr);

    return newptr;
}


/* ============================================================================
 * DEBUG AND UTILITY FUNCTIONS
 * ============================================================================ */

void ma_print_free_list(void) {
    mem_block *current = free_mem_block_list_head;
    int count = 0;

    printf("Free list:\n");
    while (current != NULL) {
        count++;
        printf("  Count %d | Block at %p | size: %zu | is_Free: %d | prev: %p | next: %p\n",
            count,
            (void *)current,
            current->size,
            current->is_Free,
            (void *)current->prev,
            (void *)current->next);
        current = current->next;
    }
    printf("\n");
}
