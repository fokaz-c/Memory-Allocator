#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "memalloc.h"

#define TEST(name) printf("\n=== %s ===\n", name)
#define PASS() printf("✓ PASS\n")
#define FAIL(msg) printf("✗ FAIL: %s\n", msg)

int main() {
    printf("======== MEMORY ALLOCATOR TEST SUITE ========\n");

    /* Test 1: Basic malloc */
    TEST("Test 1: Basic malloc");
    int *a = (int *) ma_malloc(sizeof(int));
    assert(a != NULL);
    *a = 42;
    assert(*a == 42);
    ma_free(a);
    PASS();

    /* Test 2: Multiple allocations */
    TEST("Test 2: Multiple allocations");
    int *b = (int *) ma_malloc(sizeof(int) * 2);
    assert(b != NULL);
    b[0] = 10;
    b[1] = 20;
    int *c = (int *) ma_malloc(sizeof(int) * 3);
    assert(c != NULL);
    c[0] = 30;
    c[1] = 40;
    c[2] = 50;
    assert(b[0] == 10 && b[1] == 20);
    assert(c[0] == 30 && c[1] == 40 && c[2] == 50);
    ma_free(b);
    ma_free(c);
    PASS();

    /* Test 3: Malloc after free (reuse) */
    TEST("Test 3: Malloc after free (reuse)");
    int *x = (int *) ma_malloc(sizeof(int));
    assert(x != NULL);
    *x = 100;
    ma_free(x);
    int *y = (int *) ma_malloc(sizeof(int));
    assert(y != NULL);
    *y = 200;
    assert(*y == 200);
    ma_free(y);
    PASS();

    /* Test 4: Calloc initialization */
    TEST("Test 4: Calloc initialization");
    int *arr = (int *) ma_calloc(5, sizeof(int));
    assert(arr != NULL);
    for (int i = 0; i < 5; i++) {
        assert(arr[i] == 0);
    }
    arr[0] = 99;
    assert(arr[0] == 99);
    ma_free(arr);
    PASS();

    /* Test 5: Calloc vs malloc */
    TEST("Test 5: Calloc vs malloc");
    int *malloc_test = (int *) ma_malloc(3 * sizeof(int));
    int *calloc_test = (int *) ma_calloc(3, sizeof(int));
    assert(malloc_test != NULL && calloc_test != NULL);
    for (int i = 0; i < 3; i++) {
        assert(calloc_test[i] == 0);
    }
    ma_free(malloc_test);
    ma_free(calloc_test);
    PASS();

    /* Test 6: Calloc with zero elements */
    TEST("Test 6: Calloc with zero elements");
    int *zero_alloc = (int *) ma_calloc(0, sizeof(int));
    assert(zero_alloc == NULL);
    PASS();

    /* Test 7: Realloc grow */
    TEST("Test 7: Realloc grow");
    int *grow = (int *) ma_malloc(2 * sizeof(int));
    assert(grow != NULL);
    grow[0] = 111;
    grow[1] = 222;
    grow = (int *) ma_realloc(grow, 5 * sizeof(int));
    assert(grow != NULL);
    assert(grow[0] == 111 && grow[1] == 222);
    grow[2] = 333;
    grow[3] = 444;
    grow[4] = 555;
    assert(grow[2] == 333 && grow[4] == 555);
    ma_free(grow);
    PASS();

    /* Test 8: Realloc shrink */
    TEST("Test 8: Realloc shrink");
    int *shrink = (int *) ma_malloc(10 * sizeof(int));
    assert(shrink != NULL);
    for (int i = 0; i < 10; i++) {
        shrink[i] = i * 10;
    }
    shrink = (int *) ma_realloc(shrink, 3 * sizeof(int));
    assert(shrink != NULL);
    assert(shrink[0] == 0 && shrink[1] == 10 && shrink[2] == 20);
    ma_free(shrink);
    PASS();

    /* Test 9: Realloc with NULL (like malloc) */
    TEST("Test 9: Realloc with NULL");
    int *realloc_null = (int *) ma_realloc(NULL, 4 * sizeof(int));
    assert(realloc_null != NULL);
    realloc_null[0] = 1;
    realloc_null[1] = 2;
    realloc_null[2] = 3;
    realloc_null[3] = 4;
    assert(realloc_null[3] == 4);
    ma_free(realloc_null);
    PASS();

    /* Test 10: Realloc with size 0 (like free) */
    TEST("Test 10: Realloc with size 0");
    int *realloc_free = (int *) ma_malloc(5 * sizeof(int));
    assert(realloc_free != NULL);
    realloc_free = (int *) ma_realloc(realloc_free, 0);
    assert(realloc_free == NULL);
    PASS();

    /* Test 11: Large allocation */
    TEST("Test 11: Large allocation");
    char *large = (char *) ma_malloc(10000);
    assert(large != NULL);
    memset(large, 'A', 10000);
    for (int i = 0; i < 10000; i++) {
        assert(large[i] == 'A');
    }
    ma_free(large);
    PASS();

    /* Test 12: Free NULL (should not crash) */
    TEST("Test 12: Free NULL");
    ma_free(NULL);
    PASS();

    /* Test 13: Fragmentation and coalescing */
    TEST("Test 13: Fragmentation and coalescing");
    int *p1 = (int *) ma_malloc(100);
    int *p2 = (int *) ma_malloc(100);
    int *p3 = (int *) ma_malloc(100);
    assert(p1 != NULL && p2 != NULL && p3 != NULL);
    ma_free(p2);  /* Free middle block */
    int *p4 = (int *) ma_malloc(100);  /* Should reuse p2's space */
    assert(p4 != NULL);
    ma_free(p1);
    ma_free(p3);
    ma_free(p4);
    PASS();

    /* Test 14: Realloc with data preservation */
    TEST("Test 14: Realloc with data preservation");
    char *preserve = (char *) ma_malloc(50);
    memset(preserve, 'X', 50);
    preserve = (char *) ma_realloc(preserve, 200);
    for (int i = 0; i < 50; i++) {
        assert(preserve[i] == 'X');
    }
    ma_free(preserve);
    PASS();

    /* Test 15: Mixed operations */
    TEST("Test 15: Mixed operations");
    int *mix1 = (int *) ma_malloc(10);
    int *mix2 = (int *) ma_calloc(5, sizeof(int));
    int *mix3 = (int *) ma_realloc(ma_malloc(20), 40);
    assert(mix1 != NULL && mix2 != NULL && mix3 != NULL);
    ma_free(mix1);
    ma_free(mix2);
    ma_free(mix3);
    PASS();

    printf("\n======== ALL TESTS PASSED ========\n");
    return 0;
}
