#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "memalloc.h"

int main() {
    printf("=== Testing ma_malloc ===\n");
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

    printf("\n=== Testing ma_calloc ===\n");

    // Test 1: Basic calloc allocation
    int *arr1 = (int *) ma_calloc(5, sizeof(int));
    printf("Allocated arr1 (%p) with ma_calloc(5, sizeof(int))\n", arr1);
    printf("Values (should all be 0): ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", arr1[i]);
    }
    printf("\n");
    print_free_list();

    // Test 2: Write to calloc'd memory
    arr1[0] = 10;
    arr1[4] = 50;
    printf("After writing: arr1[0]=%d, arr1[4]=%d\n", arr1[0], arr1[4]);

    // Test 3: Single element calloc
    int *single = (int *) ma_calloc(1, sizeof(int));
    printf("Allocated single (%p) with ma_calloc(1, sizeof(int)), value: %d\n", single, *single);
    print_free_list();

    // Test 4: Large calloc
    char *bytes = (char *) ma_calloc(100, sizeof(char));
    printf("Allocated bytes (%p) with ma_calloc(100, 1)\n", bytes);
    int all_zero = 1;
    for (int i = 0; i < 100; i++) {
        if (bytes[i] != 0) {
            all_zero = 0;
            break;
        }
    }
    printf("All 100 bytes are zero: %s\n", all_zero ? "YES" : "NO");
    print_free_list();

    // Test 5: Edge case - zero elements
    int *zero_test = (int *) ma_calloc(0, sizeof(int));
    printf("ma_calloc(0, sizeof(int)) returned: %p (should be NULL)\n", zero_test);

    // Test 6: Compare malloc vs calloc
    printf("\n=== Comparing malloc vs calloc ===\n");
    int *malloc_arr = (int *) ma_malloc(3 * sizeof(int));
    int *calloc_arr = (int *) ma_calloc(3, sizeof(int));
    printf("malloc'd array (%p): %d, %d, %d (may contain garbage)\n",
           malloc_arr, malloc_arr[0], malloc_arr[1], malloc_arr[2]);
    printf("calloc'd array (%p): %d, %d, %d (should be zeros)\n",
           calloc_arr, calloc_arr[0], calloc_arr[1], calloc_arr[2]);

    // Cleanup
    ma_free(arr1);
    ma_free(single);
    ma_free(bytes);
    ma_free(malloc_arr);
    ma_free(calloc_arr);
    printf("\nFreed all calloc test allocations\n");
    print_free_list();

    return 0;
}
