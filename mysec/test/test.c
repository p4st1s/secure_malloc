#include <criterion/criterion.h>
#include <stdio.h>
#include <string.h>
#include "my_secmalloc.private.h"
#include "my_secmalloc.h"
#include <sys/mman.h>
#include <time.h>
#include "utils.h"


// // Simple mmap test
// Test(mmap, simple) {
//     my_log("\n\n\n#TEST : 1\n");
//     void *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
//     cr_expect(ptr != NULL);
//     int res = munmap(ptr, 4096);
//     cr_expect(res == 0);
// }

// // Basic allocation and deallocation tests
// Test(my_malloc, basic_allocation) {
//     my_log("\n\n\n#TEST : 2\n");
//     void *ptr = my_malloc(4096);
//     cr_expect(ptr != NULL);
//     my_free(ptr);
//     my_log_meta_chunck_all();
// }

// Test(my_malloc, allocation_and_write) {
//     my_log("\n\n\n#TEST : 3\n");
//     void *ptr = my_malloc(4096);
//     cr_expect(ptr != NULL);
//     strncpy(ptr, "Hello", 6);
//     cr_expect(strcmp(ptr, "Hello") == 0);
//     my_free(ptr);
// }

// Test(my_malloc, multiple_allocations) {
//     my_log("\n\n\n#TEST : 4\n");
//     void *ptr1 = my_malloc(1000);
//     void *ptr2 = my_malloc(1000);
//     void *ptr3 = my_malloc(1000);
//     void *ptr5 = my_malloc(5000);
//     void *ptr4 = my_malloc(1000);
//     cr_expect(ptr1 != NULL);
//     cr_expect(ptr2 != NULL);
//     cr_expect(ptr3 != NULL);
//     cr_expect(ptr4 != NULL);
//     cr_expect(ptr5 != NULL);
//     check_all_canari();
//     my_free(ptr1);
//     my_free(ptr2);
//     my_free(ptr3);
//     my_log_meta_chunck_all();

//     my_free(ptr4);
//     my_free(ptr5);
//     my_log_meta_chunck_all();
// }

// Test(my_malloc, fragmentation) {
//     my_log("\n\n\n#TEST : 5\n");
//     void *ptr1 = my_malloc(1000);
//     // void *ptr2 = my_malloc(4096);
//     void *ptr3 = my_malloc(1000);
//     cr_expect(ptr1 != NULL);
//     // cr_expect(ptr2 != NULL);
//     cr_expect(ptr3 != NULL);
//     strncpy(ptr1, "Hello", 6);
//     // strncpy(ptr2, "World", 6);
//     strncpy(ptr3, "!", 2);
//     cr_expect(strcmp(ptr1, "Hello") == 0);
//     // cr_expect(strcmp(ptr2, "World") == 0);
//     cr_expect(strcmp(ptr3, "!") == 0);
//     my_free(ptr3);
//     // my_free(ptr2);
//     my_free(ptr1);
// }

// Test(my_malloc, reallocate_memory) {
//     my_log("\n\n\n#TEST : 6\n");
//     void *ptr = my_malloc(100);
//     void *new_ptr = my_realloc(ptr, 200);
//     cr_expect(new_ptr != NULL);
//     my_free(new_ptr);
// }

// Test(my_calloc, allocate_and_zero_memory) {
//     my_log("\n\n\n#TEST : 7\n");
//     void *ptr = my_calloc(10, 20);
//     cr_expect(ptr != NULL);
//     for (size_t i = 0; i < 10 * 20; i++) {
//         cr_expect(((char *)ptr)[i] == 0);
//     }
//     my_free(ptr);
// }

// // Metadata tests

// // Empty block search test
// Test(find_free_chunck, find_block) {
//     my_log("\n\n\n#TEST : 9\n");
//     void *ptr1 = my_malloc(104);
//     void *ptr2 = my_malloc(200);
//     my_free(ptr1);
//     meta_struck *empty_block = find_free_chunck(100);
//     cr_expect(empty_block != NULL);
//     cr_expect(empty_block->is_free == 1);
//     cr_expect(empty_block->sz_size >= 100);
//     my_free(ptr2);
// }

// // Stress tests
// Test(my_malloc, random_allocations) {
//     srand(time(NULL));
//     int num_allocs = rand() % 10 + 1;
//     void **ptrs = malloc(num_allocs * sizeof(void*));
//     for (int i = 0; i < num_allocs; i++) {
//         int size = rand() % 4096 + 1;
//         my_log_meta_chunck_all();
//         // my_log("######################## Allocating %d bytes\n", size);
//         ptrs[i] = my_malloc(size);

//         cr_expect(ptrs[i] != NULL);
//     }
//     for (int i = 0; i < num_allocs; i++) {
//         my_free(ptrs[i]);
//     }
//     free(ptrs);
// }

// Test(my_malloc, random_allocations_with_frees) {
//     srand(time(NULL));
//     int num_allocs = rand() % 100 + 1;
//     void **ptrs = malloc(num_allocs * sizeof(void*));
//     bool *freed = malloc(num_allocs * sizeof(bool));
//     for (int i = 0; i < num_allocs; i++) {
//         int size = rand() % 4096 + 1;
//         ptrs[i] = my_malloc(size);
//         freed[i] = false;
//         cr_expect(ptrs[i] != NULL);
//         if (rand() % 2 == 0 && i > 0) {
//             int j;
//             do {
//                 j = rand() % i;
//             } while (freed[j]);
//             my_free(ptrs[j]);
//             freed[j] = true;
//         }
//     }
//     for (int i = 0; i < num_allocs; i++) {
//         if (!freed[i]) {
//             my_free(ptrs[i]);
//         }
//     }
//     my_free(ptrs);
//     my_free(freed);
// }

// // Security-focused tests
// Test(my_malloc, buffer_overflow_detection) {
//     my_log("\n\n\n#TEST : 10\n");
//     void *ptr = my_malloc(100);
//     cr_expect(ptr != NULL);
//     char *data = (char *)ptr;
//     for (int i = 0; i < 150; i++) {
//         data[i] = 'A';  // Write past the allocated memory
//     }
//     // Ideally, here we should have a way to detect the overflow
//     my_free(ptr);
// }

// Test(my_malloc, double_free_detection) {
//     my_log("\n\n\n#TEST : 11\n");
//     void *ptr = my_malloc(100);
//     cr_expect(ptr != NULL);
//     my_free(ptr);
//     // Double free
//     my_free(ptr);
//     // Ideally, here we should have a way to detect the double free
// }

// Test(my_malloc, use_after_free_detection) {
//     my_log("\n\n\n#TEST : 12\n");
//     void *ptr = my_malloc(100);
//     cr_expect(ptr != NULL);
//     my_free(ptr);
//     char *data = (char *)ptr;
//     strcpy(data, "Use after free");
//     // Ideally, here we should have a way to detect the use after free
// }

// Test(my_malloc, memory_leak_detection) {
//     my_log("\n\n\n#TEST : 13\n");
//     void *ptr = my_malloc(100);
//     cr_expect(ptr != NULL);
//     // mygetlist();
//     // Memory leak
//     // Ideally, here we should have a way to detect the memory leak
// }

// Test (my_free, mergefree){
//     void *ptr = my_malloc(100);
//     void *ptr2 = my_malloc(100);
//     void *ptr3 = my_malloc(100);
//     void *ptr4 = my_malloc(100);
//     my_free(ptr);
//     my_free(ptr2);
//     // mygetlist();
//     my_free(ptr3);
//     my_free(ptr4);
// }


// Test(memory, my_malloc) {
//     void *ptr = my_malloc(100);
//     cr_assert_not_null(ptr, "my_malloc should not return NULL");
//     meta_struck *chunk = find_chunck(ptr);
//     cr_assert_not_null(chunk, "The chunk metadata should not be NULL");
//     cr_assert(chunk->sz_size >= 100, "The chunk size should be at least 100 bytes");
//     cr_assert_eq(chunk->is_free, 0, "The chunk should be marked as not free");
// }

// Test(memory, my_free) {
//     void *ptr = my_malloc(100);
//     cr_assert_not_null(ptr, "my_malloc should not return NULL");

//     my_free(ptr);
//     meta_struck *chunk = find_chunck(ptr);
//     cr_assert_not_null(chunk, "The chunk metadata should not be NULL");
//     cr_assert_eq(chunk->is_free, 1, "The chunk should be marked as free after my_free");
// }

// Test(memory, my_realloc) {
//     void *ptr = my_malloc(100);
//     cr_assert_not_null(ptr, "my_malloc should not return NULL");

//     void *new_ptr = my_realloc(ptr, 200);
//     cr_assert_not_null(new_ptr, "my_realloc should not return NULL");

//     meta_struck *chunk = find_chunck(new_ptr);
//     cr_assert_not_null(chunk, "The chunk metadata should not be NULL");
//     cr_assert(chunk->sz_size>= 200, "The chunk size should be at least 200 bytes");
    
//     cr_assert_eq(chunk->is_free, 0, "The chunk should be marked as not free after my_realloc");
// }

// Test(memory, my_calloc) {
//     void *ptr = my_calloc(10, 50);
//     cr_assert_not_null(ptr, "my_calloc should not return NULL");

//     meta_struck *chunk = find_chunck(ptr);

//     cr_assert_not_null(chunk, "The chunk metadata should not be NULL");
//     cr_assert(chunk->sz_size>= 10 * 50, "The chunk size should be at least 500 bytes");
//     cr_assert_eq(chunk->is_free, 0, "The chunk should be marked as not free");
//     // Check if the allocated memory is zero-initialized
//     for (size_t i = 0; i < 10 * 50; ++i) {
//         cr_assert_eq(((char *)ptr)[i], 0, "Memory allocated by my_calloc should be zero-initialized");
//     }
// }

// Test(memory, random_allocations) {
//     srand(time(NULL));
//     int num_allocs = rand() % 2000 + 1;
//     void **ptrs = malloc(num_allocs * sizeof(void*));
//     for (int i = 0; i < num_allocs; i++) {
//         int size = rand() % 4096 + 1;
//         ptrs[i] = my_malloc(size);

//         cr_expect(ptrs[i] != NULL);
//     }
//     for (int i = 0; i < num_allocs; i++) {
//         my_free(ptrs[i]);
//     }
//     free(ptrs);
// }

// Test(memory, my_malloc_zero_size) {
//     void *ptr = my_malloc(0);
//     cr_assert_null(ptr, "my_malloc with size 0 should return NULL");
// }

// Test(memory, my_realloc_null_ptr) {
//     void *new_ptr = my_realloc(NULL, 100);
//     cr_assert_not_null(new_ptr, "my_realloc with NULL pointer should return a valid pointer");
// }

// Test(memory, my_realloc_zero_size) {
//     void *ptr = my_malloc(100);
//     cr_assert_not_null(ptr, "my_malloc should not return NULL");

//     void *new_ptr = my_realloc(ptr, 0);
//     cr_assert_null(new_ptr, "my_realloc with size 0 should return NULL");
// }

// Test(memory, my_realloc_invalid_ptr) {
//     void *ptr = (void *)0x12345678; // Some invalid address
//     void *new_ptr = my_realloc(ptr, 100);
//     cr_assert_null(new_ptr, "my_realloc with invalid pointer should return NULL");
// }

Test(memory,my_realloc_opti){
    void *ptr =my_malloc(1000);
    my_log_meta_chunck_all();

    void *ptr2 = my_realloc(ptr,2000);    
    my_log_meta_chunck_all();


    cr_assert_eq(ptr,ptr2,"my_realloc error ptr not equalls");

}

// Test(memory, my_malloc_overflow) {
//     void *XORcBVjbwEUiEA = my_malloc(77);
// XORcBVjbwEUiEA = my_realloc(XORcBVjbwEUiEA, 80);
// void *ifarxRPiOCA = my_malloc(164);
// my_free(ifarxRPiOCA);
// void *HLRKRfnPEZA = my_malloc(46);
// my_free(HLRKRfnPEZA);
// void *EFxFmZPXBg = my_malloc(237);
// void *mZTbrxgDFTw = my_malloc(171);
// my_free(mZTbrxgDFTw);
// void *qtOvhLEpKqcw = my_malloc(193);
// void *gFlCYVSGJnzQ = my_malloc(159);
// void *OxqyKOfgymANQ = my_malloc(101);
// EFxFmZPXBg = my_realloc(EFxFmZPXBg, 36);
// gFlCYVSGJnzQ = my_realloc(gFlCYVSGJnzQ, 117);
// OxqyKOfgymANQ = my_realloc(OxqyKOfgymANQ, 70);
// qtOvhLEpKqcw = my_realloc(qtOvhLEpKqcw, 241);
// void *fjkSQXKyUDw = my_malloc(185);
// my_free(fjkSQXKyUDw);
// void *mKNSUFSRw = my_malloc(172);
// void *uFBSBAouhxtrw = my_malloc(178);
// my_free(mKNSUFSRw);
// uFBSBAouhxtrw = my_realloc(uFBSBAouhxtrw, 124);
// void *uBXqYeWRXZA = my_malloc(11);
// my_free(uBXqYeWRXZA);
// void *CoFPMeLcuwhA = my_malloc(114);
// my_free(CoFPMeLcuwhA);
// void *ifzOnYaNpjQA = my_malloc(68);
// ifzOnYaNpjQA = my_realloc(ifzOnYaNpjQA, 154);
// void *jnoZmjAMg = my_malloc(116);
// void *uYbFedOoPbA = my_malloc(189);
// my_free(uYbFedOoPbA);
// jnoZmjAMg = my_realloc(jnoZmjAMg, 29);
// void *NJLFUktQ = my_malloc(167);
// my_free(NJLFUktQ);
// void *KWrhndJMxHnUA = my_malloc(106);
// KWrhndJMxHnUA = my_realloc(KWrhndJMxHnUA, 247);
// void *NqMffyCWvA = my_malloc(165);
// my_free(NqMffyCWvA);
// void *kaDtnNiMg = my_malloc(20);
// my_free(kaDtnNiMg);
// void *dWyllqkSDA = my_malloc(164);
// my_free(dWyllqkSDA);
// void *zZydvLBqg = my_malloc(254);
// zZydvLBqg = my_realloc(zZydvLBqg, 61);
// void *IhNiMQNjJJFw = my_malloc(212);
// void *DtRanillAVew = my_malloc(182);
// void *LPqTjsRPg = my_malloc(84);
// void *gHheAZpruvjA = my_malloc(223);
// my_free(gHheAZpruvjA);
// IhNiMQNjJJFw = my_realloc(IhNiMQNjJJFw, 14);
// LPqTjsRPg = my_realloc(LPqTjsRPg, 56);
// void *cCkVCGVMzumow = my_malloc(83);
// my_free(DtRanillAVew);
// void *mECiktwyFw = my_malloc(42);
// my_free(mECiktwyFw);
// cCkVCGVMzumow = my_realloc(cCkVCGVMzumow, 123);
// void *ZeKIvBVhw = my_malloc(132);
// void *WdydAPnqburGg = my_malloc(37);
// my_free(WdydAPnqburGg);
// ZeKIvBVhw = my_realloc(ZeKIvBVhw, 96);
// void *DjhTqMag = my_malloc(43);
// void *lvWGPqDUg = my_malloc(125);
// my_free(lvWGPqDUg);
// void *doUJYFHkYKVcA = my_malloc(166);
// my_free(doUJYFHkYKVcA);
// DjhTqMag = my_realloc(DjhTqMag, 106);
// void *FHiuuUNCBqjA = my_malloc(214);
// void *woORQdEEpDSQ = my_malloc(40);
// FHiuuUNCBqjA = my_realloc(FHiuuUNCBqjA, 153);
// void *YqAlAXEHFLJg = my_malloc(126);
// my_free(YqAlAXEHFLJg);
// woORQdEEpDSQ = my_realloc(woORQdEEpDSQ, 119);
// }

