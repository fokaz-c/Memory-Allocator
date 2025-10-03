#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "memalloc.h"

int main() {
    int *a = (int *) ma_malloc(sizeof(int));
    *a = 1;
    printf("Allocated a (%p) with value %d\n", a, *a);
    print_free_list();

    int *b = (int *) ma_malloc(sizeof(int) * 2);
    b[0] = 2; b[1] = 3;
    printf("Allocated b (%p) with values %d, %d\n", b, b[0], b[1]);
    print_free_list();

    int *c = (int *) ma_malloc(sizeof(int) * 3);
    c[0] = 4; c[1] = 5; c[2] = 6;
    printf("Allocated c (%p) with values %d, %d, %d\n", c, c[0], c[1], c[2]);
    print_free_list();

    ma_free(b);
    printf("Freed b (%p)\n", b);
    print_free_list();

    ma_free(a);
    printf("Freed a (%p)\n", a);
    print_free_list();

    ma_free(c);
    printf("Freed c (%p)\n", c);
    print_free_list();

    c = ma_malloc((size_t)sizeof(int));
    print_free_list();
    return 0;
}
