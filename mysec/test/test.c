#include <criterion/criterion.h>
#include <stdio.h>
#include <string.h>
#include "my_secmalloc.private.h"
#include "my_secmalloc.h"
#include <sys/mman.h>
#include <time.h>

// Simple mmap test
Test(mmap, simple) {
    my_log("\n\n\n#TEST : 1\n");
    void *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    cr_expect(ptr != NULL);
    int res = munmap(ptr, 4096);
    cr_expect(res == 0);
}

// Basic allocation and deallocation tests
Test(my_malloc, basic_allocation) {
    my_log("\n\n\n#TEST : 2\n");
    void *ptr = my_malloc(4096);
    cr_expect(ptr != NULL);
    my_free(ptr);
}

Test(my_malloc, allocation_and_write) {
    my_log("\n\n\n#TEST : 3\n");
    void *ptr = my_malloc(4096);
    cr_expect(ptr != NULL);
    strncpy(ptr, "Hello", 6);
    cr_expect(strcmp(ptr, "Hello") == 0);
    my_free(ptr);
}

Test(my_malloc, multiple_allocations) {
    my_log("\n\n\n#TEST : 4\n");
    void *ptr1 = my_malloc(1000);
    void *ptr2 = my_malloc(1000);
    void *ptr3 = my_malloc(1000);
    void *ptr4 = my_malloc(1000);
    cr_expect(ptr1 != NULL);
    cr_expect(ptr2 != NULL);
    cr_expect(ptr3 != NULL);
    cr_expect(ptr4 != NULL);
    my_free(ptr1);
    my_free(ptr2);
    my_free(ptr3);
    my_free(ptr4);
}

Test(my_malloc, fragmentation) {
    my_log("\n\n\n#TEST : 5\n");
    void *ptr1 = my_malloc(1000);
    void *ptr2 = my_malloc(4096);
    void *ptr3 = my_malloc(1000);
    cr_expect(ptr1 != NULL);
    cr_expect(ptr2 != NULL);
    cr_expect(ptr3 != NULL);
    strncpy(ptr1, "Hello", 6);
    strncpy(ptr2, "World", 6);
    strncpy(ptr3, "!", 2);
    cr_expect(strcmp(ptr1, "Hello") == 0);
    cr_expect(strcmp(ptr2, "World") == 0);
    cr_expect(strcmp(ptr3, "!") == 0);
    my_free(ptr1);
    my_free(ptr2);
    my_free(ptr3);
}

Test(my_malloc, reallocate_memory) {
    my_log("\n\n\n#TEST : 6\n");
    void *ptr = my_malloc(100);
    void *new_ptr = my_realloc(ptr, 200);
    cr_expect(new_ptr != NULL);
    my_free(new_ptr);
}

Test(my_calloc, allocate_and_zero_memory) {
    my_log("\n\n\n#TEST : 7\n");
    void *ptr = my_calloc(10, 20);
    cr_expect(ptr != NULL);
    for (size_t i = 0; i < 10 * 20; i++) {
        cr_expect(((char *)ptr)[i] == 0);
    }
    my_free(ptr);
}

// Metadata tests
Test(init_metadata, initialize) {
    my_log("\n\n\n#TEST : 8\n");
    void *ptr = init_metadata();
    cr_expect(ptr != NULL);
}

// Empty block search test
Test(find_empty_block, find_block) {
    my_log("\n\n\n#TEST : 9\n");
    init_metadata();
    void *ptr1 = my_malloc(100);
    void *ptr2 = my_malloc(200);
    my_free(ptr1);
    meta_struck *empty_block = find_empty_block(100);
    cr_expect(empty_block != NULL);
    cr_expect(empty_block->is_free == 1);
    cr_expect(empty_block->sz_size >= 100);
    my_free(ptr2);
}

// Stress tests
Test(my_malloc, random_allocations) {
    srand(time(NULL));
    int num_allocs = rand() % 100 + 1;
    void **ptrs = malloc(num_allocs * sizeof(void*));
    for (int i = 0; i < num_allocs; i++) {
        int size = rand() % 4096 + 1;
        ptrs[i] = my_malloc(size);
        cr_expect(ptrs[i] != NULL);
    }
    for (int i = 0; i < num_allocs; i++) {
        my_free(ptrs[i]);
    }
    free(ptrs);
}

Test(my_malloc, random_allocations_with_frees) {
    srand(time(NULL));
    int num_allocs = rand() % 100 + 1;
    void **ptrs = malloc(num_allocs * sizeof(void*));
    bool *freed = malloc(num_allocs * sizeof(bool));
    for (int i = 0; i < num_allocs; i++) {
        int size = rand() % 4096 + 1;
        ptrs[i] = my_malloc(size);
        freed[i] = false;
        cr_expect(ptrs[i] != NULL);
        if (rand() % 2 == 0 && i > 0) {
            int j;
            do {
                j = rand() % i;
            } while (freed[j]);
            my_free(ptrs[j]);
            freed[j] = true;
        }
    }
    for (int i = 0; i < num_allocs; i++) {
        if (!freed[i]) {
            my_free(ptrs[i]);
        }
    }
    free(ptrs);
    free(freed);
}

// Security-focused tests
Test(my_malloc, buffer_overflow_detection) {
    my_log("\n\n\n#TEST : 10\n");
    void *ptr = my_malloc(100);
    cr_expect(ptr != NULL);
    char *data = (char *)ptr;
    for (int i = 0; i < 150; i++) {
        data[i] = 'A';  // Write past the allocated memory
    }
    // Ideally, here we should have a way to detect the overflow
    my_free(ptr);
}

Test(my_malloc, double_free_detection) {
    my_log("\n\n\n#TEST : 11\n");
    void *ptr = my_malloc(100);
    cr_expect(ptr != NULL);
    my_free(ptr);
    // Double free
    my_free(ptr);
    // Ideally, here we should have a way to detect the double free
}

Test(my_malloc, use_after_free_detection) {
    my_log("\n\n\n#TEST : 12\n");
    void *ptr = my_malloc(100);
    cr_expect(ptr != NULL);
    my_free(ptr);
    char *data = (char *)ptr;
    strcpy(data, "Use after free");
    // Ideally, here we should have a way to detect the use after free
}

Test(my_malloc, memory_leak_detection) {
    my_log("\n\n\n#TEST : 13\n");
    void *ptr = my_malloc(100);
    cr_expect(ptr != NULL);
    // mygetlist();
    // Memory leak
    // Ideally, here we should have a way to detect the memory leak
}