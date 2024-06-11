#include <criterion/criterion.h>
#include <stdio.h>
#include <string.h>
#include "my_secmalloc.private.h"
#include "my_secmalloc.h"
#include <sys/mman.h>
#include <time.h>
// Test(mmap, simple) {
//     my_log("\n\n\n#TEST : 1\n");
//     void *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
//     cr_expect(ptr != NULL);
//     int res = munmap(ptr, 4096);
//     cr_expect(res == 0);
// }

// Test(my_malloc, simple) {
//     my_log("\n\n\n#TEST : 2\n");
//     void *ptr = my_malloc(4096);
//     cr_expect(ptr != NULL);
//     int res = munmap(ptr, 4096);
//     cr_expect(res == 0);
// }

// Test(init_metadata, simple2) {
//     my_log("\n\n\n#TEST : 3\n");    
//     void *ptr = init_metadata();
//     cr_expect(ptr != NULL);
// }

// Test(my_malloc, simple2) {
//     my_log("\n\n\n#TEST : 4\n"); 
//     void *ptr = my_malloc(4096);
//     cr_expect(ptr != NULL);
//     strncpy(ptr, "Hello", 6);
//     cr_expect(strcmp(ptr, "Hello") == 0);
//     int res = munmap(ptr, 4096);
//     cr_expect(res == 0);  

// }
// Test(my_malloc, simple3) {
//     my_log("\n\n\n#TEST : 5\n"); 
//     void *ptr = my_malloc(1000);
//     cr_expect(ptr != NULL);
//     strncpy(ptr, "Hello", 6);
//     cr_expect(strcmp(ptr, "Hello") == 0);
//     void *ptr2 = my_malloc(1000);
//     cr_expect(ptr2 != NULL);
//     strncpy(ptr2, "World", 6);
//     cr_expect(strcmp(ptr2, "World") == 0);
// }

// Test(my_malloc, simple4) {
//     my_log("\n\n\n#TEST : 6\n"); 
//     void *ptr = my_malloc(1000);
//     cr_expect(ptr != NULL);
//     strncpy(ptr, "Hello", 6);
//     cr_expect(strcmp(ptr, "Hello") == 0);
//     void *ptr2 = my_malloc(4096);
//     cr_expect(ptr2 != NULL);
//     strncpy(ptr2, "World", 6);
//     cr_expect(strcmp(ptr2, "World") == 0);

// }

// Test(my_malloc, simple5) {
//     my_log("\n\n\n#TEST : 7\n"); 
//     void *ptr = my_malloc(1000);
//     cr_expect(ptr != NULL);
//     strncpy(ptr, "Hello", 6);
//     cr_expect(strcmp(ptr, "Hello") == 0);
//     void *ptr2 = my_malloc(4096);
//     cr_expect(ptr2 != NULL);
//     strncpy(ptr2, "World", 6);
//     void *ptr3 = my_malloc(1000);
//     cr_expect(ptr3 != NULL);
//     strncpy(ptr3, "totoo", 6);
//     cr_expect(strcmp(ptr3, "totoo") == 0);
//     cr_expect(strcmp(ptr2, "World") == 0);
// }

// Test(my_malloc,simple6){
//     my_log("\n\n\n#TEST : 8\n");
//     void *ptr = my_malloc(1000);
//     void *ptr2 = my_malloc(4096);
//     void *ptr3 = my_malloc(1000);
//     void *ptr4 = my_malloc(1000);
//     cr_expect(ptr != NULL);
//     cr_expect(ptr2 != NULL);
//     cr_expect(ptr3 != NULL);
//     cr_expect(ptr4 != NULL);
// }

// Test (my_malloc, simple7){
//     my_log("\n\n\n#TEST : 9\n");
//     void *ptr = my_malloc(1000);
//     void *ptr3 = my_malloc(1000);
//     void *ptr4 = my_malloc(1000);
//     void *ptr2 = my_malloc(1000);

//     void *ptr6 = my_malloc(1000);
//     my_log("J'AIIIIIIIIIIIIIIIIIIIIIIIIIIiii TOUUUUUUUUUUUUUUUUUUUUUUT FAITTTTTTTTTTTTTTTTTTTTT\n");
       
//     // void *ptr5 = my_malloc(8000);
//     // void *ptr4 = my_malloc(1000);
//     cr_expect(ptr2 != NULL);
//     cr_expect(ptr != NULL);
//     cr_expect(ptr3 != NULL);
//     cr_expect(ptr4 != NULL);
//     cr_expect(ptr6 != NULL);
//     // cr_expect(ptr4 != NULL);
//     // cr_expect(ptr5 != NULL);
//     my_log("\n\nFINAL\n\n");
//     mygetlist();
// }



// Test(my_malloc, allocate_memory) {
//     void *ptr = my_malloc(100);
//     cr_expect(ptr != NULL, "my_malloc failed to allocate memory");
//     my_free(ptr);
// }

// Test(my_realloc, reallocate_memory) {
//     void *ptr = my_malloc(100);
//     void *new_ptr = my_realloc(ptr, 200);
//     cr_expect(new_ptr != NULL, "my_realloc failed to reallocate memory");
//     my_free(new_ptr);
// }

// Test(my_calloc, allocate_and_zero_memory) {
//     void *ptr = my_calloc(10, 20);
//     cr_expect(ptr != NULL, "my_calloc failed to allocate memory");
//     for (size_t i = 0; i < 10 * 20; i++) {
//         cr_expect(((char *)ptr)[i] == 0, "my_calloc did not zero the memory");
//     }
//     my_free(ptr);
// }

// Test(memory_allocation, multiple_allocations) {
//     void *ptr1 = my_malloc(50);
//     cr_expect(ptr1 != NULL, "First my_malloc failed");

//     void *ptr2 = my_malloc(100);
//     cr_expect(ptr2 != NULL, "Second my_malloc failed");

//     my_free(ptr1);
//     my_free(ptr2);
// }

// // Test(init_metadata, initialize) {
// //     void *ptr = init_metadata();
// //     cr_expect(ptr != NULL, "init_metadata failed to initialize");

// //     // Check the metadata head
// //     cr_expect(ptr_metahead != NULL, "Metadata head is NULL");
// //     cr_expect(ptr_metahead->is_free == 1, "Metadata head is not free");
// //     cr_expect(ptr_metahead->sz_size == 4096, "Metadata head size is incorrect");
// //     cr_expect(ptr_metahead->p_ptr_data != NULL, "Metadata head data pointer is NULL");
// // }

// Test(find_empty_block, find_block) {
//     init_metadata();
//     void *ptr1 = my_malloc(100);
//     void *ptr2 = my_malloc(200);

//     my_free(ptr1);

//     meta_struck *empty_block = find_empty_block(100);
//     cr_expect(empty_block != NULL, "find_empty_block did not find the empty block");
//     cr_expect(empty_block->is_free == 1, "The found block is not free");
//     cr_expect(empty_block->sz_size >= 100, "The found block size is not sufficient");

//     my_free(ptr2);
// }

// Test (my_malloc, filldata){
//     void *ptr = my_malloc(6);
//     cr_expect(ptr != NULL);
//     strncpy(ptr, "Hello", 6);
//     cr_expect(strcmp(ptr, "Hello") == 0);
//     my_free(ptr);
// }

// Test (my_free, simple){
//     my_log("\n\n\n#TEST : 9\n");
//     void *ptr = my_malloc(1000);
//     void *ptr3 = my_malloc(1000);
//     void *ptr4 = my_malloc(1000);
//     void *ptr2 = my_malloc(1000);

//     void *ptr6 = my_malloc(4000);
       
//     // void *ptr5 = my_malloc(8000);
//     // void *ptr4 = my_malloc(1000);
//     cr_expect(ptr2 != NULL);
//     cr_expect(ptr != NULL);
//     cr_expect(ptr3 != NULL);
//     cr_expect(ptr4 != NULL);
//     cr_expect(ptr6 != NULL);
//     // cr_expect(ptr4 != NULL);
//     // cr_expect(ptr5 != NULL);
    
//     my_free(ptr3);
//     my_free(ptr4);
    
//     mygetlist();
// }

// Test(my_malloc, random) {
//     srand(time(NULL));  // seed the random number generator
//     int num_allocs = rand() % 100 + 1;  // random number of allocations between 1 and 100
//     void **ptrs = malloc(num_allocs * sizeof(void*));  // array to hold pointers to allocated memory

//     for (int i = 0; i < num_allocs; i++) {
//         int size = rand() %10000;  // random size between 1 and 4096
//         ptrs[i] = my_malloc(size);
//         cr_expect(ptrs[i] != NULL);
//     }

//     for (int i = 0; i < num_allocs; i++) {
//         my_free(ptrs[i]);
//     }

//     free(ptrs);
// }
// Test(my_malloc, random_with_frees) {
//     srand(time(NULL));  // seed the random number generator
//     int num_allocs = rand() % 100 + 1;  // random number of allocations between 1 and 100
//     void **ptrs = malloc(num_allocs * sizeof(void*));  // array to hold pointers to allocated memory
//     bool *freed = malloc(num_allocs * sizeof(bool));  // array to track which allocations have been freed

//     for (int i = 0; i < num_allocs; i++) {
//         int size = rand() % 4096 + 1;  // random size between 1 and 4096
//         ptrs[i] = my_malloc(size);
//         freed[i] = false;  // initially, no allocations have been freed
//         cr_expect(ptrs[i] != NULL);

//         // Randomly decide whether to free an allocation
//         if (rand() % 2 == 0 && i > 0) {  // don't try to free if this is the first allocation
//             int j;
//             do {
//                 j = rand() % i;  // choose a random allocation to free
//             } while (freed[j]);  // keep choosing until we find one that hasn't been freed yet
//             my_free(ptrs[j]);
//             freed[j] = true;
//         }
//     }

//     // Free any remaining allocations
//     for (int i = 0; i < num_allocs; i++) {
//         if (!freed[i]) {
//             my_free(ptrs[i]);
//         }
//     }

//     free(ptrs);
//     free(freed);
// }


Test (my_malloc, simple){
    void *ptr = my_malloc(1000);
    void *ptr2 = my_malloc(1000);
    void *ptr3 = my_malloc(1000);
    void *ptr4 = my_malloc(1000);
    void *ptr5 = my_malloc(1000);
    void *ptr6 = my_malloc(1000);
    void *ptr7 = my_malloc(1000);
    void *ptr8 = my_malloc(1000);

    cr_expect(ptr != NULL);
    cr_expect(ptr2 != NULL);
    cr_expect(ptr3 != NULL);
    cr_expect(ptr4 != NULL);
    cr_expect(ptr5 != NULL);
    cr_expect(ptr6 != NULL);
    cr_expect(ptr7 != NULL);
    cr_expect(ptr8 != NULL);

    my_free(ptr);
    my_free(ptr2);
    my_free(ptr6);
    my_free(ptr8);
    mygetlist();
    void *ptr9 = my_malloc(2000);
    cr_expect(ptr9 != NULL);
    mygetlist();
}