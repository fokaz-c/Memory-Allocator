#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>
#include "memalloc.h"

#define BLUE    "\x1b[34m"
#define GREEN   "\x1b[32m"
#define RED     "\x1b[31m"
#define RESET   "\x1b[0m"

#define BURST_ALLOC_COUNT 5000
#define THREAD_COUNT 4
#define THREAD_ALLOC_COUNT 500

// --- Timer ---
double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

// --- Helper for printing metrics ---
void print_metrics(const char *label, double t_ma, double t_std) {
    if (t_std <= 0) return;
    const char *color = (t_ma > t_std) ? RED : GREEN;
    printf(BLUE "%s" RESET ": %s%.4f ms" RESET " | stdlib: %.4f ms\n",
           label, color, t_ma, t_std);
}

// --- Test Cases ---

void test_basic_malloc(void) {
    double t1 = get_time_ms();
    int *a = (int *) ma_malloc(sizeof(int));
    double t2 = get_time_ms();
    ma_free(a);

    double t1_std = get_time_ms();
    int *a_std = (int *) malloc(sizeof(int));
    double t2_std = get_time_ms();
    free(a_std);

    print_metrics("malloc(1 alloc, 4 bytes)", t2 - t1, t2_std - t1_std);
}

void test_multiple_allocations(void) {
    double t1 = get_time_ms();
    int *b = (int *) ma_malloc(sizeof(int) * 2);
    int *c = (int *) ma_malloc(sizeof(int) * 3);
    double t2 = get_time_ms();
    ma_free(b);
    ma_free(c);

    double t1_std = get_time_ms();
    int *b_std = (int *) malloc(sizeof(int) * 2);
    int *c_std = (int *) malloc(sizeof(int) * 3);
    double t2_std = get_time_ms();
    free(b_std);
    free(c_std);

    print_metrics("malloc(2 allocs, 8/12 bytes)", t2 - t1, t2_std - t1_std);
}

void test_reuse_after_free(void) {
    double t1 = get_time_ms();
    int *x = (int *) ma_malloc(sizeof(int));
    ma_free(x);
    int *y = (int *) ma_malloc(sizeof(int));
    double t2 = get_time_ms();
    ma_free(y);

    double t1_std = get_time_ms();
    int *x_std = (int *) malloc(sizeof(int));
    free(x_std);
    int *y_std = (int *) malloc(sizeof(int));
    double t2_std = get_time_ms();
    free(y_std);

    print_metrics("malloc(reuse)", t2 - t1, t2_std - t1_std);
}

void test_calloc(void) {
    double t1 = get_time_ms();
    int *arr = (int *) ma_calloc(5, sizeof(int));
    double t2 = get_time_ms();
    ma_free(arr);

    double t1_std = get_time_ms();
    int *arr_std = (int *) calloc(5, sizeof(int));
    double t2_std = get_time_ms();
    free(arr_std);

    print_metrics("calloc(5 elements, 4 bytes each)", t2 - t1, t2_std - t1_std);
}

void test_realloc_grow(void) {
    double t1 = get_time_ms();
    int *grow = (int *) ma_malloc(2 * sizeof(int));
    grow = (int *) ma_realloc(grow, 5 * sizeof(int));
    double t2 = get_time_ms();
    ma_free(grow);

    double t1_std = get_time_ms();
    int *grow_std = (int *) malloc(2 * sizeof(int));
    grow_std = (int *) realloc(grow_std, 5 * sizeof(int));
    double t2_std = get_time_ms();
    free(grow_std);

    print_metrics("realloc(grow 8 -> 20 bytes)", t2 - t1, t2_std - t1_std);
}

void test_realloc_shrink(void) {
    double t1 = get_time_ms();
    int *shrink = (int *) ma_malloc(10 * sizeof(int));
    shrink = (int *) ma_realloc(shrink, 3 * sizeof(int));
    double t2 = get_time_ms();
    ma_free(shrink);

    double t1_std = get_time_ms();
    int *shrink_std = (int *) malloc(10 * sizeof(int));
    shrink_std = (int *) realloc(shrink_std, 3 * sizeof(int));
    double t2_std = get_time_ms();
    free(shrink_std);

    print_metrics("realloc(shrink 40 -> 12 bytes)", t2 - t1, t2_std - t1_std);
}

void test_large_allocation(void) {
    double t1 = get_time_ms();
    char *large = (char *) ma_malloc(10000);
    double t2 = get_time_ms();
    ma_free(large);

    double t1_std = get_time_ms();
    char *large_std = (char *) malloc(10000);
    double t2_std = get_time_ms();
    free(large_std);

    print_metrics("malloc(1 alloc, 10KB)", t2 - t1, t2_std - t1_std);
}

void test_burst_allocation(void) {
    double t1 = get_time_ms();
    void **ptrs = (void **) ma_malloc(BURST_ALLOC_COUNT * sizeof(void *));
    for (int i = 0; i < BURST_ALLOC_COUNT; i++) ptrs[i] = ma_malloc(64);
    for (int i = 0; i < BURST_ALLOC_COUNT; i++) ma_free(ptrs[i]);
    ma_free(ptrs);
    double t2 = get_time_ms();

    double t1_std = get_time_ms();
    void **ptrs_std = (void **) malloc(BURST_ALLOC_COUNT * sizeof(void *));
    for (int i = 0; i < BURST_ALLOC_COUNT; i++) ptrs_std[i] = malloc(64);
    for (int i = 0; i < BURST_ALLOC_COUNT; i++) free(ptrs_std[i]);
    free(ptrs_std);
    double t2_std = get_time_ms();

    print_metrics("malloc(5000 allocs, 64 bytes)", t2 - t1, t2_std - t1_std);
}

// --- Multithreading ---

typedef struct { int alloc_count; void **ptrs; } thread_data;

void *thread_func(void *arg) {
    thread_data *data = (thread_data *)arg;
    data->ptrs = (void **) ma_malloc(data->alloc_count * sizeof(void *));
    for (int i = 0; i < data->alloc_count; i++) data->ptrs[i] = ma_malloc(128);
    for (int i = 0; i < data->alloc_count; i++) ma_free(data->ptrs[i]);
    ma_free(data->ptrs);
    return NULL;
}

void *thread_func_std(void *arg) {
    thread_data *data = (thread_data *)arg;
    data->ptrs = (void **) malloc(data->alloc_count * sizeof(void *));
    for (int i = 0; i < data->alloc_count; i++) data->ptrs[i] = malloc(128);
    for (int i = 0; i < data->alloc_count; i++) free(data->ptrs[i]);
    free(data->ptrs);
    return NULL;
}

void test_multithreaded_allocation(void) {
    pthread_t threads[THREAD_COUNT];
    thread_data thread_args[THREAD_COUNT];

    double t1 = get_time_ms();
    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_args[i] = (thread_data){ .alloc_count = THREAD_ALLOC_COUNT };
        pthread_create(&threads[i], NULL, thread_func, &thread_args[i]);
    }
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    double t2 = get_time_ms();

    double t1_std = get_time_ms();
    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_args[i] = (thread_data){ .alloc_count = THREAD_ALLOC_COUNT };
        pthread_create(&threads[i], NULL, thread_func_std, &thread_args[i]);
    }
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    double t2_std = get_time_ms();

    const char *color = (t2 - t1 > t2_std - t1_std) ? RED : GREEN;
    printf(BLUE "malloc(%d threads, %d allocs each)" RESET ": %s%.4f ms" RESET " | stdlib: %.4f ms\n",
           THREAD_COUNT, THREAD_ALLOC_COUNT, color, t2 - t1, t2_std - t1_std);
}

// --- Main ---

int main() {
    test_basic_malloc();
    test_multiple_allocations();
    test_reuse_after_free();
    test_calloc();
    test_realloc_grow();
    test_realloc_shrink();
    test_large_allocation();
    test_burst_allocation();
    test_multithreaded_allocation();
    return 0;
}
