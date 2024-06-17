#define _GNU_SOURCE
#include <stdio.h>
#include <alloca.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>



#include "my_secmalloc.private.h"
#include "utils.h"



#define LOG_ENTER_FUNCTION() my_log("[INFO] Entering function %s\n", __func__)
#define LOG_EXIT_FUNCTION() my_log("[INFO] Exiting function %s\n", __func__)


// ################ GLobal variable #################
#define METADATA_SIZE 100000
#define PADDING 1000000
#define PAGE_SIZE 4096
#define CANARY_SIZE 4
meta_struck *ptr_metahead = NULL;
meta_struck *ptr_metatail = NULL;
int nb_metadata = 0;

// ################ Function #################

/**
 * Initializes a memory pool at the specified address with the given size.
 *
 * @param addr The starting address of the memory pool.
 * @param size The size of the memory pool in bytes.
 * @return A pointer to the initialized memory pool, or NULL if initialization fails.
 */
void *init_pool(void *addr, size_t size)
{
    void *pool = (meta_struck *)mmap(addr,size,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANON,-1,0);
    if (pool == MAP_FAILED)
    {
        my_log("[ERROR] Allocating the pool of size : %zu\n", size);
        return NULL;
    }
    my_log("[INFO] Pool of size %zu allocated at %p\n", size, pool);
    return pool;
}

/**
 * Initializes the memory pools and metadata for secure memory allocation.
 * 
 * @return A pointer to the first metadata structure in the pool, or NULL if initialization fails.
 */
void *init_all(){
    // Register cleanup functions to be called at program exit
    atexit(close_logging);
    atexit(check_memory_leak);
    atexit(clean_memory);
    init_logging();
    
    // Initialize the metadata pool
    void* ptr = init_pool(NULL, sizeof(meta_struck) * METADATA_SIZE);
    if (ptr == NULL)
    {
        my_log("[ERROR] Initialisation of the metadata pool\n");
        return NULL;
    }
    
    nb_metadata ++;
    
    // Initialize the data pool
    void *ptr_data = init_pool(ptr+PADDING, PAGE_SIZE-CANARY_SIZE);
    if (ptr_data == NULL)
    {
        my_log("[ERROR] Initialisation of the data pool\n");
        munmap(ptr_metahead, sizeof(meta_struck) * METADATA_SIZE);
        return NULL;
    }
    
    // Create the first metadata structure
    meta_struck *ptr_first = (meta_struck *)ptr;
    ptr_first->p_next = NULL;
    ptr_first->sz_size = PAGE_SIZE-CANARY_SIZE;
    ptr_first->p_ptr_data = ptr_data;
    ptr_first->is_free = 1;
    
    // Generate and store a random canary value
    uint32_t canari = get_random_canary();
    ptr_first->canari = canari;
    uint32_t *canari_addr = ptr_first->p_ptr_data+ptr_first->sz_size;
    *canari_addr = canari;

    my_log("[INFO] canary vs canari_addr %d vs %d\n", canari, *canari_addr);
    
    // Update the metadata head and tail pointers
    ptr_metahead = ptr_first;
    ptr_metatail = ptr_first;
    nb_metadata ++;
    
    // Log the metadata chunk
    my_log_meta_chunck(ptr_first);
    
    return ptr_first;
}

/**
 * Finds a free chunk of memory that is large enough to accommodate the requested size.
 * 
 * @param size The size of the memory chunk to be allocated.
 * @return A pointer to the metadata structure of the found free chunk, or NULL if no suitable chunk is found.
 */
meta_struck *find_free_chunck(size_t size){
    LOG_ENTER_FUNCTION();
    meta_struck *ptr = ptr_metahead;
    while (ptr != NULL)
    {
        if (ptr->is_free == 1 && ptr->sz_size-CANARY_SIZE >= size)
        {
            LOG_EXIT_FUNCTION();
            return ptr;
        }
        ptr = ptr->p_next;
    }
    LOG_EXIT_FUNCTION();
    return NULL;
}

/**
 * @brief Allocates a chunk of memory of the specified size.
 *
 * This function searches for a free chunk of memory of the specified size. If a suitable chunk is found,
 * it is returned. Otherwise, a new chunk is allocated using the alloc_chunk() function. If the size of the
 * found chunk is larger than the requested size, the chunk is split into two parts using the split_chunk()
 * function.
 *
 * @param size The size of the memory chunk to allocate.
 * @return A pointer to the allocated memory chunk, or NULL if allocation fails.
 */
void *get_chunck(size_t size)
{
    LOG_ENTER_FUNCTION();
    meta_struck *ptr_free = find_free_chunck(size);
    if (ptr_free == NULL)
    {
        ptr_free = alloc_chunck(size);
    }
    else if (ptr_free->sz_size-CANARY_SIZE >= size)
    {
        ptr_free = split_chunck(ptr_free, size);
    } 
    LOG_EXIT_FUNCTION();
    return ptr_free;
}

meta_struck * find_chunck(void *ptr){
    meta_struck *ptr_meta = ptr_metahead;
    while (ptr_meta != NULL)
    {
        if (ptr_meta->p_ptr_data == ptr)
        {
            return ptr_meta;
        }
        ptr_meta = ptr_meta->p_next;
    }
    return NULL;
}


/**
 * Splits a chunk of memory into two parts, creating a new metadata structure for the second part.
 * 
 * @param ptr A pointer to the metadata structure of the chunk to be split.
 * @param size The size of the first part of the chunk after splitting.
 * @return A pointer to the data portion of the first part of the chunk.
 */
void *split_chunck(meta_struck *ptr, size_t size){
    LOG_ENTER_FUNCTION();
    my_log("[INFO] Splitting chunck\n");
    my_log_meta_chunck(ptr);
    meta_struck *ptr_empty = (meta_struck *)(ptr_metahead+(nb_metadata++*sizeof(meta_struck)));
    if (ptr->sz_size-(CANARY_SIZE*2) <= size)
    {
        ptr->is_free = 0;
        return ptr->p_ptr_data;
    }
    uint32_t canari = get_random_canary();
    ptr_empty->p_ptr_data = ptr->p_ptr_data + size+CANARY_SIZE;
    ptr_empty->p_next = ptr->p_next;
    ptr_empty->sz_size = ptr->sz_size - (size+sizeof(canari));
    ptr_empty->is_free = 1;
    ptr_empty->canari = ptr->canari;
    uint32_t  *canari_addr = ptr->p_ptr_data+size;
    *canari_addr = canari;
    my_log("[INFO] Canari is %d\n", *canari_addr);
    my_log("[INFO] Canari addr is %p\n", canari_addr);
    ptr->sz_size = size;
    ptr->p_next = ptr_empty;
    ptr->is_free = 0;
    ptr->canari = canari;
    

    if (ptr == ptr_metatail)
    {
        ptr_metatail = ptr_empty;
    }

    LOG_EXIT_FUNCTION();
    return ptr->p_ptr_data;
}


/**
 * @brief Allocates a chunk of memory with the specified size.
 *
 * This function allocates a chunk of memory with the specified size and performs necessary bookkeeping operations.
 *
 * @param size The size of the chunk to allocate.
 * @return A pointer to the allocated memory chunk, or NULL if allocation fails.
 */
void *alloc_chunck(size_t size){
    LOG_ENTER_FUNCTION();
    meta_struck *ptr = (meta_struck *)(ptr_metahead+(nb_metadata++*sizeof(meta_struck)));
    void *ptr_data = init_pool((meta_struck*)ptr_metahead+ (sizeof(meta_struck)* METADATA_SIZE), size+CANARY_SIZE);
    if (ptr_data == NULL)
    {
        my_log("[ERROR] Allocating the chunck of size : %zu\n", size);
        return NULL;
    }
    ptr->p_ptr_data = ptr_data;
    ptr->p_next = NULL;
    ptr->sz_size = size;
    ptr->is_free = 0;
    uint32_t canari = get_random_canary();
    ptr->canari = canari;
    uint32_t *canari_addr = ptr->p_ptr_data+ptr->sz_size;
    *canari_addr = canari;
    ptr_metatail->p_next = ptr;
    ptr_metatail = ptr;
    if ((size+CANARY_SIZE % PAGE_SIZE)!= 0){
        meta_struck *ptr_empty = (meta_struck *)(ptr_metahead+(nb_metadata++*sizeof(meta_struck)));
        ptr_empty->p_ptr_data = ptr->p_ptr_data + size+CANARY_SIZE;
        ptr_empty->p_next = NULL;
        ptr_empty->sz_size = PAGE_SIZE - (((size+CANARY_SIZE)%PAGE_SIZE)+CANARY_SIZE);
        ptr_empty->is_free = 1;
        uint32_t canari = get_random_canary();
        ptr_empty->canari = canari;
        my_log_meta_chunck(ptr_empty);
        uint32_t *canari_addr = ptr_empty->p_ptr_data+ptr_empty->sz_size;
        *canari_addr = canari;
        
        ptr_metatail->p_next = ptr_empty;
        ptr_metatail = ptr_empty;
        }
    my_log_meta_chunck(ptr);
    my_log_meta_chunck(ptr_metatail);

    LOG_EXIT_FUNCTION();
    return ptr->p_ptr_data;
}


/**
 * Frees the memory block pointed to by `ptr`.
 * 
 * This function marks the memory block as free and performs necessary operations
 * such as logging and merging adjacent free blocks.
 * 
 * @param ptr A pointer to the memory block to be freed.
 */
void my_free(void *ptr){
    LOG_ENTER_FUNCTION();
    meta_struck *ptr_meta = ptr_metahead;
    while (ptr_meta != NULL)
    {
        if (ptr_meta->p_ptr_data == ptr)
        {
            ptr_meta->is_free = 1;
            my_log_meta_chunck(ptr_meta);
            break;
        }
        ptr_meta = ptr_meta->p_next;
    }
    check_all_canari();
    my_merge();
    my_log_meta_chunck_all();
    LOG_EXIT_FUNCTION();
}
/**
 * Merges adjacent free memory chunks in the memory pool.
 * This function iterates through the metadata structures and merges
 * consecutive free memory chunks that are adjacent to each other.
 * It updates the size and canary value of the merged chunk.
 * Finally, it updates the metadata tail pointer if necessary.
 */
void my_merge(){
    LOG_ENTER_FUNCTION();
    // my_log("#############################################################\n");
    my_log_meta_chunck_all();
    meta_struck *tmp2 = ptr_metahead;
    while (tmp2 != NULL)
    {
        meta_struck *tmp = tmp2;
        size_t size = tmp->sz_size;
        while (tmp->is_free == 1 && tmp->p_next != NULL && tmp->p_next->is_free == 1 && tmp->p_ptr_data+tmp->sz_size+CANARY_SIZE == tmp->p_next->p_ptr_data)
        {
            size += tmp->p_next->sz_size+CANARY_SIZE;
            tmp->canari = tmp->p_next->canari;
            tmp = tmp->p_next;
            
        }
        tmp2->p_next = tmp->p_next;
        tmp2->sz_size = size;
        tmp2->canari = tmp->canari;
        if (tmp->p_next == NULL)
        {
            ptr_metatail = tmp2;
        }
        tmp2 = tmp2->p_next;
    }
    // my_log("#############################################################\n");
    my_log_meta_chunck_all();
    LOG_EXIT_FUNCTION();

}

/**
 * Logs the details of a metadata chunk.
 *
 * This function logs the details of a metadata chunk, including its pointer,
 * pointer to data, size, free status, next pointer, and canary value.
 *
 * @param meta A pointer to the metadata chunk.
 */
void my_log_meta_chunck (meta_struck *meta){
    my_log("\t[DEBUG] Metadata details: \n\
            \t\t[*] ptr=%p, \n\
            \t\t[*] p_ptr_data=%p, \n\
            \t\t[*] sz_size=%lu, \n\
            \t\t[*] is_free=%d, \n\
            \t\t[*] p_next=%p\n\
            \t\t[*] canari=%d\n",
            meta, meta->p_ptr_data, meta->sz_size, meta->is_free, meta->p_next, meta->canari);

}

/**
 * Logs the metadata details of all chunks.
 * This function iterates through the linked list of metadata structures and logs the details of each chunk.
 */
void my_log_meta_chunck_all(){
    LOG_ENTER_FUNCTION();
    my_log("[INFO] Metadata details: \n");
    // my_log("------------------------------------\n");
    meta_struck *ptr = ptr_metahead;
    while (ptr != NULL)
    {
        my_log_meta_chunck(ptr);
        ptr = ptr->p_next;
    }
    LOG_EXIT_FUNCTION();
}

/**
 * Checks if the canary value of a given memory chunk is valid.
 * If the canary value is not valid, an error message is logged.
 *
 * @param ptr A pointer to the meta_struck structure representing the memory chunk.
 */
void check_canari(meta_struck *ptr){
    LOG_ENTER_FUNCTION();
    if (ptr == NULL)
    {
        my_log("[ERROR] ptr is NULL\n");
        LOG_EXIT_FUNCTION();
        return;
    }
    my_log_meta_chunck(ptr);
    uint32_t *canari_addr = ptr->p_ptr_data+ptr->sz_size;
    my_log("[INFO] Canari vs canari_addr %d vs %d\n", ptr->canari, *canari_addr);
    if (*canari_addr != ptr->canari)
    {
        my_log("[ERROR] Canari is not the same\n");
        my_log_meta_chunck(ptr);
    }
    LOG_EXIT_FUNCTION();
}

/**
 * Checks the canary values for all allocated memory blocks.
 * This function iterates through the linked list of meta structures
 * and calls the check_canari() function for each structure.
 */
void check_all_canari(){
    LOG_ENTER_FUNCTION();
    meta_struck *ptr = ptr_metahead;
    while (ptr != NULL)
    {
        check_canari(ptr);
        ptr = ptr->p_next;
    }
    LOG_EXIT_FUNCTION();
}

/**
 * Checks for memory leaks in the allocated memory blocks.
 * This function iterates through the linked list of metadata structures
 * and checks if any memory block is not freed.
 * If a memory block is found to be not freed, an error message is logged.
 */
void check_memory_leak()
{
    LOG_ENTER_FUNCTION();
    meta_struck *ptr = ptr_metahead;
    while (ptr != NULL)
    {
        if (ptr->is_free == 0)
        {
            my_log("[ERROR] Memory leak\n");
            my_log_meta_chunck(ptr);
        }
        ptr = ptr->p_next;
    }
    LOG_EXIT_FUNCTION();
}

/**
 * @brief Cleans up the allocated memory and resets global variables.
 * 
 * This function frees the memory allocated by the data pool and resets the global variables
 * used by the memory management system.
 */
void clean_memory()
{
    // Get total size of the data pool
    // size_t data_size = 0;
    meta_struck *tmp = ptr_metahead;
    while (tmp != NULL)
    {
        munmap(tmp->p_ptr_data, tmp->sz_size+CANARY_SIZE);
        tmp = tmp->p_next;
    }

    // Free pools
    // munmap(ptr_metahead, data_size);
    munmap(ptr_metahead, METADATA_SIZE * sizeof(meta_struck));

    // Reset the global variables
    ptr_metahead = NULL;
    ptr_metatail = NULL;
    nb_metadata = 0;
}




// ################ Function demandé #################
/**
 * @brief Allocates memory of the specified size using a custom memory allocation function.
 *
 * This function allocates memory of the specified size and logs the allocation information.
 * It ensures that the allocated memory is aligned to a multiple of 16 bytes.
 *
 * @param size The size of the memory to allocate.
 * @return A pointer to the allocated memory, or NULL if the allocation fails.
 */
void *my_malloc(size_t size){
    my_log("[INFO] Allocating memory of size %lu\n", size);
    LOG_ENTER_FUNCTION();
    if (size <= 0)
    {
        my_log("[ERROR] size is less than 0\n");
        LOG_EXIT_FUNCTION();
        return NULL;
    }
    size=(size % 16 ? size + 16 - (size % 16) : size);
    if (ptr_metahead == NULL)
    {
        my_log("[INFO] First malloc\n");
        void *ptr = init_all();
        if (ptr == NULL)
        {
            my_log("[ERROR] ptr is NULL\n");
            LOG_EXIT_FUNCTION();
            return NULL;
        }
    }
    // allocation of data 
    void *ptr_data = get_chunck(size);
    if (ptr_data == NULL)
    {
        my_log("[ERROR] ptr_data is NULL\n");
        LOG_EXIT_FUNCTION();
        return NULL;
    }
    LOG_EXIT_FUNCTION();
    return ptr_data;
}







/**
 * Allocates memory for an array of elements, each with a specified size, and initializes them to zero.
 *
 * @param nmemb The number of elements to allocate.
 * @param size The size of each element.
 * @return A pointer to the allocated memory, or NULL if the allocation fails.
 */
void *my_calloc(size_t nmemb, size_t size){
    LOG_ENTER_FUNCTION();
    my_log("[INFO] Allocating memory of size %lu\n", size);
    void *ptr = my_malloc(nmemb*size);
    if (ptr == NULL)
    {
        my_log("[ERROR] ptr is NULL\n");
        LOG_EXIT_FUNCTION();
        return NULL;
    
    }
    memset(ptr, 0, nmemb*size);
    LOG_EXIT_FUNCTION();
    return ptr;
}



/**
 * Reallocates a block of memory pointed to by `ptr` to a new size specified by `size`.
 * 
 * @param ptr   Pointer to the block of memory to be reallocated.
 * @param size  New size of the memory block in bytes.
 * @return      Pointer to the reallocated memory block, or NULL if the reallocation fails.
 */
void *my_realloc(void *ptr, size_t size){
    LOG_ENTER_FUNCTION();
    if (size <= 0)
    {
        my_log("[ERROR] size is less than 0\n");
        LOG_EXIT_FUNCTION();
        return NULL;
    }
    my_log("[INFO] Reallocating memory of size %lu\n", size);
    if (ptr == NULL)
    {
        my_log("[ERROR] ptr is NULL\n");
        LOG_EXIT_FUNCTION();
        return my_malloc(size);
    }
    meta_struck *ptr_meta = ptr_metahead;
    while (ptr_meta != NULL)
    {
        if (ptr_meta->p_ptr_data == ptr)
        {
            break;
        }
        ptr_meta = ptr_meta->p_next;
    }
    if (ptr_meta == NULL)
    {
        my_log("[ERROR] ptr_meta is NULL\n");
        LOG_EXIT_FUNCTION();
        return NULL;
    }
    if (ptr_meta->sz_size >= size)
    {
        LOG_EXIT_FUNCTION();
        return ptr;
    }

    size_t total = ptr_meta->sz_size;
    meta_struck *tmp = ptr_meta;


    while(tmp->p_next->is_free == 1 && tmp->p_ptr_data+tmp->sz_size+CANARY_SIZE == tmp->p_next->p_ptr_data)
    {
        total += tmp->p_next->sz_size+CANARY_SIZE;
        my_log("[INFO] Total is %lu\n", total);
        if (total >= size)
        {
            break;
        }
        
        tmp = tmp->p_next;



    }
    my_log("[INFO] Total is %lu\n", total);
        //todo 
    if (total>= size){
        ptr_meta->p_next= tmp->p_next->p_next;
        ptr_meta->sz_size = total;
        ptr_meta->canari= tmp->canari;
        
        return split_chunck(ptr_meta, size);

    }

    else{
        
        void *ptr_new = my_malloc(size);
        if (ptr_new == NULL)
        {
            my_log("[ERROR] ptr_new is NULL\n");
            LOG_EXIT_FUNCTION();
            return NULL;
        }
        memcpy(ptr_new, ptr, ptr_meta->sz_size);
        my_free(ptr);
    LOG_EXIT_FUNCTION();
    return ptr_new;
    }
}


#ifdef DYNAMIC

/**
 * Custom implementation of the malloc function.
 * Allocates a block of memory of the given size.
 *
 * @param size The size of the memory block to allocate.
 * @return A pointer to the allocated memory block, or NULL if the allocation fails.
 */
void *malloc(size_t size)
{
    return my_malloc(size);
}

/**
 * Custom implementation of the free function.
 * Frees the memory block pointed to by the given pointer.
 *
 * @param ptr A pointer to the memory block to free.
 */
void free(void *ptr)
{
    my_free(ptr);
}

/**
 * Custom implementation of the calloc function.
 * Allocates a block of memory for an array of nmemb elements, each of size bytes,
 * and initializes all bytes to zero.
 *
 * @param nmemb The number of elements in the array.
 * @param size The size of each element in bytes.
 * @return A pointer to the allocated memory block, or NULL if the allocation fails.
 */
void *calloc(size_t nmemb, size_t size)
{
    return my_calloc(nmemb, size);
}

/**
 * Custom implementation of the realloc function.
 * Changes the size of the memory block pointed to by the given pointer to the given size.
 *
 * @param ptr A pointer to the memory block to reallocate.
 * @param size The new size of the memory block.
 * @return A pointer to the reallocated memory block, or NULL if the reallocation fails.
 */
void *realloc(void *ptr, size_t size)
{
    return my_realloc(ptr, size);
}

#endif