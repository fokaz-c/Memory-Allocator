#include "memalloc.h"
#include "memalloc_internal.h"
#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define HEAP_SIZE (size_t)(1024 * 1024)

static char *heap_pool = NULL;
static size_t heap_offset = 0;
static mem_block *free_mem_block_list_head = NULL;

static pthread_mutex_t allocator_mutex;
static pthread_once_t mutex_init_once = PTHREAD_ONCE_INIT;

static void init_mutex(void) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&allocator_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

static void init_heap(void) {
    if (heap_pool != NULL) {
        return;
    }

    heap_pool = (char *) mmap(
        NULL,
        HEAP_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );

    if (heap_pool == MAP_FAILED) {
        heap_pool = NULL;
        perror("mmap failed");
    }
}

static void cleanup_heap(void) {
    if (heap_pool != NULL) {
        munmap(heap_pool, HEAP_SIZE);
        heap_pool = NULL;
        heap_offset = 0;
    }
}

static bool are_adjacent(mem_block *first, mem_block *second) {
    void *end_of_first = (char *)first + sizeof(mem_block) + first->size;
    return end_of_first == second;
}

static void coalesce_with_neighbors(mem_block *block) {
    mem_block *current = free_mem_block_list_head;

    while (current != NULL) {
        mem_block *next_iter = current->next;

        if (current == block) {
            current = next_iter;
            continue;
        }

        if (are_adjacent(block, current)) {
            block->size += sizeof(mem_block) + current->size;
            remove_from_free_mem_list(current);
        } else if (are_adjacent(current, block)) {
            current->size += sizeof(mem_block) + block->size;

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

void *ma_malloc(size_t size) {
    pthread_once(&mutex_init_once, init_mutex);
    pthread_mutex_lock(&allocator_mutex);

    if (heap_pool == NULL) {
        init_heap();
    }

    if (heap_pool == NULL) {
        pthread_mutex_unlock(&allocator_mutex);
        return NULL;
    }

    if (size == 0 || size > SIZE_MAX - sizeof(mem_block)) {
        pthread_mutex_unlock(&allocator_mutex);
        return NULL;
    }

    mem_block *block = find_free_block(size);

    if (block != NULL) {
        remove_from_free_mem_list(block);
        split_block(block, size);
        pthread_mutex_unlock(&allocator_mutex);
        return (void *)((char *)block + sizeof(mem_block));
    }

    if (heap_offset + sizeof(mem_block) + size > HEAP_SIZE) {
        pthread_mutex_unlock(&allocator_mutex);
        return NULL;
    }

    mem_block *new_block = (mem_block *)&heap_pool[heap_offset];
    mem_block_init(new_block, size);
    new_block->is_Free = false;

    heap_offset += sizeof(mem_block) + size;

    pthread_mutex_unlock(&allocator_mutex);
    return (void *)((char *)new_block + sizeof(mem_block));
}

void ma_free(void *ptr) {
    pthread_once(&mutex_init_once, init_mutex);
    pthread_mutex_lock(&allocator_mutex);

    if (ptr == NULL) {
        pthread_mutex_unlock(&allocator_mutex);
        return;
    }

    mem_block *block = (mem_block *)((char *)ptr - sizeof(mem_block));
    add_to_free_mem_block_list(block);

    pthread_mutex_unlock(&allocator_mutex);
}

void *ma_calloc(size_t n, size_t size) {
    pthread_once(&mutex_init_once, init_mutex);
    pthread_mutex_lock(&allocator_mutex);

    size_t total_size = n * size;
    if (n == 0 || total_size / n != size) {
        pthread_mutex_unlock(&allocator_mutex);
        return NULL;
    }

    void *ptr = ma_malloc(total_size);
    if (ptr == NULL) {
        pthread_mutex_unlock(&allocator_mutex);
        return NULL;
    }

    memset(ptr, 0, total_size);

    pthread_mutex_unlock(&allocator_mutex);
    return ptr;
}

void *ma_realloc(void *ptr, size_t size) {
    pthread_once(&mutex_init_once, init_mutex);
    pthread_mutex_lock(&allocator_mutex);

    if (ptr == NULL) {
        void *result = ma_malloc(size);
        pthread_mutex_unlock(&allocator_mutex);
        return result;
    }

    if (size == 0) {
        ma_free(ptr);
        pthread_mutex_unlock(&allocator_mutex);
        return NULL;
    }

    mem_block *current = (mem_block *)((char *)ptr - sizeof(mem_block));
    size_t old_size = current->size;

    if (size <= old_size) {
        pthread_mutex_unlock(&allocator_mutex);
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

            pthread_mutex_unlock(&allocator_mutex);
            return ptr;
        }
        free_node = free_node->next;
    }

    void *newptr = ma_malloc(size);
    if (newptr == NULL) {
        pthread_mutex_unlock(&allocator_mutex);
        return NULL;
    }

    memcpy(newptr, ptr, old_size);
    ma_free(ptr);

    pthread_mutex_unlock(&allocator_mutex);
    return newptr;
}

void ma_print_free_list(void) {
    pthread_once(&mutex_init_once, init_mutex);
    pthread_mutex_lock(&allocator_mutex);

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
    printf("Heap usage: %zu / %zu bytes (%.2f%%)\n",
           heap_offset, HEAP_SIZE, (double)heap_offset / HEAP_SIZE * 100);
    printf("\n");

    pthread_mutex_unlock(&allocator_mutex);
}

__attribute__((destructor))
static void cleanup_on_exit(void) {
    cleanup_heap();
}
