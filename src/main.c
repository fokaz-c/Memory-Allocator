#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "memalloc.h"

int main () {
    void* a = sbrk(0);
    printf("1: %p\n", a);

    void *b = malloc(6);
    printf("Malloc with 6 size: %p\n", b);

    int *c = (int *) ma_malloc(sizeof(int));
    *c = 10;
    printf("malloc using custom ma_alloc of size %d bytes: %p\n", (int)sizeof(uint64_t) , c);
    printf("size of pointer : %d\n", (int)sizeof(c));
    printf("insert 10 in %p:\n  dereferncing gives : %d\n", c, *c);

    return 0;
}
