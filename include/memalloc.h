#ifndef MEMALLOC_H
#define MEMALLOC_H

#include<stddef.h>

/*
 * @brief- The ma_malloc function allocates "size" bytes and returns a pointer to that allocated memory.
 */
void *ma_malloc(size_t size);

/*
 * @brief- The ma_calloc function allocates memory for an array of n elements, each of "size" bytes.
 */
void *ma_calloc(size_t n, size_t size);

/*
 * @brief- ma_realloc function changes the size of the memory block pointed by "ptr" to "size" bytes.
 */
void *ma_realloc(void* ptr, size_t size);

/*
 * @brief- ma_free function frees the memory space pointed by ptr.
 */
void ma_free(void *ptr);
void print_free_list();
#endif
