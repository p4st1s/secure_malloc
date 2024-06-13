#define _GNU_SOURCE
#include "my_secmalloc.private.h"
#include <stdio.h>
#include <alloca.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"


#define LOG_ENTER_FUNCTION() my_log("[INFO] Entering function %s\n", __func__)
#define LOG_EXIT_FUNCTION() my_log("[INFO] Exiting function %s\n", __func__)



// ################ GLobal variable #################
#define METADATA_SIZE 10000
#define PADDING 10000
meta_struck *ptr_metahead = NULL;
meta_struck *ptr_metatail = NULL;
int nb_meta = 0;

// ################ Function #################

/**
 * @brief Initializes the metadata for secure memory allocation.
 *
 * @return A pointer to the allocated memory, or NULL if allocation fails.
 */
void *init_metadata()
{
    LOG_ENTER_FUNCTION();
    init_logging();
    size_t meta_size = sizeof(meta_struck) * METADATA_SIZE;
    void *ptr = mmap(NULL,
                     meta_size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS,
                     -1,
                     0);
    if (ptr == MAP_FAILED)
    {
        LOG_EXIT_FUNCTION();
        return NULL;
    }
    void *ptr_data = mmap(ptr + sizeof(meta_struck) * METADATA_SIZE + PADDING,
                          4096,
                          PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS,
                          -1,
                          0);
    if (ptr_data == MAP_FAILED)
    {
        munmap(ptr, meta_size);
        LOG_EXIT_FUNCTION();
        return NULL;
    }
    meta_struck *ptr_meta = (meta_struck *)ptr;
    nb_meta++;
    ptr_meta->p_ptr_data = ptr_data;
    ptr_meta->sz_size = 4096;
    ptr_meta->is_free = 1;
    ptr_meta->p_next = NULL;
    ptr_metahead = ptr_meta;
    ptr_metatail = ptr_meta;
    my_log("[DEBUG] ptr_meta\n");
    my_whoami(ptr_meta);
    LOG_EXIT_FUNCTION();
    return ptr;
}

/**
 * @brief Finds an empty block in the memory allocation system that
 * can accommodate the given size.
 *
 * @param size The size of the block to be allocated.
 * @return A pointer to the metadata structure of the empty block,
 * or NULL if no suitable block is found.
 */
meta_struck *find_empty_block(size_t size)
{
    LOG_ENTER_FUNCTION();
    meta_struck *tmp = ptr_metahead;
    while (tmp != NULL)
    {
        if (tmp->is_free == 1 && tmp->sz_size >= size)
        {
            // my_log("[DEBUG] tmp \n");
            //  my_whoami(tmp);
            LOG_EXIT_FUNCTION();
            return tmp;
        }
        tmp = tmp->p_next;
    }
    LOG_EXIT_FUNCTION();
    return NULL;
}

/**
 * Allocates a block of memory of the given size using a secure memory
 * allocation mechanism.
 *
 * @param size The size of the memory block to allocate.
 * @return A pointer to the allocated memory block, or NULL if the
 * allocation fails.
 */
void *my_malloc(size_t size)
{
    my_log("[INFO] Allocating memory of size %lu\n", size);
    LOG_ENTER_FUNCTION();
    mygetlist();

    // Check if the size is less than or equal to 0
    if (size == 0)
    {
        my_log("[ERROR] Size is less than 0\n");
        LOG_EXIT_FUNCTION();
        return NULL;
    }

    // Check if the metadata pointer is NULL (first malloc)
    if (ptr_metahead == NULL)
    {
        my_log("[INFO] First malloc\n");
        void *ptr = init_metadata();
        if (ptr == NULL)
        {
            my_log("[ERROR] ptr is NULL\n");
            LOG_EXIT_FUNCTION();
            return NULL;
        }
    }

    // Allocate data block
    void *ptr_data = alloc_data(size);
    if (ptr_data == NULL)
    {
        my_log("[ERROR] Data allocation failed\n");
        LOG_EXIT_FUNCTION();
        return NULL;
    }

    LOG_EXIT_FUNCTION();
    return ptr_data;
}
/**
 * Allocates a block of memory of the specified size and
 * returns a pointer to the allocated memory.
 * This function is used when the allocated block is not empty.
 *
 * @param size The size of the memory block to allocate.
 * @return A pointer to the allocated memory block.
 */

void *alloc_data_notempty(size_t size)
{
    LOG_ENTER_FUNCTION();

    // Check if we need to remap the metadata pool
    if (nb_meta >= METADATA_SIZE)
    {
        size_t new_size = sizeof(meta_struck) * (METADATA_SIZE + PADDING);
        if (remap_metadata(new_size) == NULL)
        {
            LOG_EXIT_FUNCTION();
            return NULL;
        }
    }

    if (ptr_metahead == NULL)
    {
        my_log("[ERROR] ptr_metahead is NULL\n");
        LOG_EXIT_FUNCTION();
        return NULL;
    }
    meta_struck *new_ptr_meta = (meta_struck *)ptr_metahead + sizeof(meta_struck) * nb_meta;
    my_log("[DEBUG] ############################################### new_ptr_meta : %p\n", new_ptr_meta);
    nb_meta++;
    ptr_metatail->p_next = new_ptr_meta;
    my_whoami(ptr_metatail);
    void *ptr_data = mmap(ptr_metahead + sizeof(meta_struck) * METADATA_SIZE + PADDING,
                          size,
                          PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS,
                          -1,
                          0);
    if (ptr_data == MAP_FAILED)
    {
        perror("mmap");
        my_log("[ERROR] mmap failed \n");
        LOG_EXIT_FUNCTION();
        return NULL;
    }
    new_ptr_meta->p_ptr_data = ptr_data;
    new_ptr_meta->sz_size = size;
    new_ptr_meta->is_free = 0;
    if (size % 4096 == 0)
    {
        new_ptr_meta->p_next = NULL;
        ptr_metatail = new_ptr_meta;
        my_whoami(new_ptr_meta);
        mygetlist();
        LOG_EXIT_FUNCTION();
        return new_ptr_meta->p_ptr_data;
    }
    meta_struck *empty_next = (meta_struck *)ptr_metahead + sizeof(meta_struck) * nb_meta;
    my_log("[DEBUG] ############################################### empty_next %p\n", empty_next);
    nb_meta++;
    new_ptr_meta->p_next = empty_next;
    empty_next->p_ptr_data = ptr_data + size;
    empty_next->sz_size = 4096 - (size % 4096);
    empty_next->is_free = 1;
    empty_next->p_next = NULL;
    ptr_metatail = empty_next;
    my_whoami(new_ptr_meta);
    my_whoami(empty_next);
    mygetlist();
    LOG_EXIT_FUNCTION();

    return new_ptr_meta->p_ptr_data;
}

/**
 * Allocates a block of memory of the specified size and updates the metadata.
 *
 * @param size The size of the memory block to allocate.
 * @param ptr_free A pointer to the metadata of the free block.
 * @return A pointer to the allocated memory block.
 */
void *alloc_data_empty(size_t size, meta_struck *ptr_free)
{
    LOG_ENTER_FUNCTION();
    // Check if we need to remap the metadata pool
    if (nb_meta >= METADATA_SIZE)
    {
        size_t new_size = sizeof(meta_struck) * (METADATA_SIZE + PADDING);
        if (remap_metadata(new_size) == NULL)
        {
            LOG_EXIT_FUNCTION();
            return NULL;
        }
    }

    meta_struck *empty = (meta_struck *)ptr_metahead + (sizeof(meta_struck) * nb_meta);
    nb_meta++;
    empty->p_ptr_data = ptr_free->p_ptr_data + size;
    empty->sz_size = ptr_free->sz_size - size;
    empty->is_free = 1;
    empty->p_next = ptr_free->p_next;
    ptr_free->sz_size = size;
    ptr_free->is_free = 0;
    ptr_free->p_next = empty;
    if (ptr_free == ptr_metatail)
    {
        ptr_metatail = empty;
    }
    my_whoami(ptr_free);
    my_whoami(empty);
    LOG_EXIT_FUNCTION();
    return ptr_free->p_ptr_data;
}
/**
 * Allocates a block of memory of the specified size.
 *
 * @param size The size of the memory block to allocate.
 * @return A pointer to the allocated memory block, or NULL if allocation fails.
 */
void *alloc_data(size_t size)
{
    LOG_ENTER_FUNCTION();
    void *ptr_data;
    meta_struck *ptr_free = find_empty_block(size);
    if (ptr_free == NULL)
    {
        ptr_data = alloc_data_notempty(size);
        LOG_EXIT_FUNCTION();
        return ptr_data;
    }
    else
    {
        ptr_data = alloc_data_empty(size, ptr_free);
        LOG_EXIT_FUNCTION();
        return ptr_data;
    }
    LOG_EXIT_FUNCTION();

    return NULL;
}

/**
 * Frees a previously allocated memory block.
 *
 * This function marks the memory block pointed to by `ptr` as free. If the block is already free,
 * an error message is logged. After marking the block as free, the function may perform block merging
 * to optimize memory usage.
 *
 * @param ptr A pointer to the memory block to be freed.
 */
void my_free(void *ptr)
{
    my_log("[INFO] Freeing memory block at %p\n", ptr);
    LOG_ENTER_FUNCTION();
    mygetlist();
    bool is_found = false;
    meta_struck *tmp = ptr_metahead;
    while (tmp != NULL)
    {
        if (tmp->p_ptr_data == ptr)
        {
            is_found = true;
            if (tmp->is_free == 1)
            {
                my_log("[ERROR] Block is already free\n");
                break;
            }
            tmp->is_free = 1;
            break;
        }
        tmp = tmp->p_next;
    }
    if (!is_found)
    {
        my_log("[ERROR] Block not found\n");
        LOG_EXIT_FUNCTION();
        return;
    }
    // merging blocks
    my_merge();
}

/**
 * Merges consecutive free memory blocks in the memory pool.
 * This function iterates through the memory blocks and merges any adjacent blocks that are both free.
 * It updates the size of the current block by adding the size of the next block and
 * removes the next block from the linked list.
 * This process continues until there are no more adjacent free blocks to merge.
 */
void my_merge()
{
    LOG_ENTER_FUNCTION();
    mygetlist();
    meta_struck *tmp2 = ptr_metahead;
    while (tmp2 != NULL)
    {
        meta_struck *tmp = tmp2;
        size_t size = tmp->sz_size;
        while (tmp->is_free == 1 && tmp->p_next != NULL && tmp->p_next->is_free == 1)
        {
            size += tmp->p_next->sz_size;
            tmp = tmp->p_next;
        }
        tmp2->p_next = tmp->p_next;
        tmp2->sz_size = size;
        if (tmp->p_next == NULL)
        {
            ptr_metatail = tmp2;
        }
        tmp2 = tmp2->p_next;
    }
    mygetlist();
    LOG_EXIT_FUNCTION();
}

/**
 * Retrieves the metadata structure associated with a given pointer.
 *
 * @param ptr The pointer whose metadata structure needs to be retrieved.
 * @return A pointer to the metadata structure if found, otherwise NULL.
 */
meta_struck *get_chunck(void *ptr)
{
    LOG_ENTER_FUNCTION();
    meta_struck *tmp = ptr_metahead;
    while (tmp != NULL)
    {
        if (tmp->p_ptr_data == ptr)
        {
            LOG_EXIT_FUNCTION();
            return tmp;
        }
        tmp = tmp->p_next;
    }
    LOG_EXIT_FUNCTION();
    return NULL;
}

/**
 * Reallocates a memory block with a new size.
 *
 * @param ptr   Pointer to the memory block to be reallocated.
 * @param size  New size for the memory block.
 * @return      Pointer to the reallocated memory block, or NULL if the reallocation failed.
 */
void *my_realloc(void *ptr, size_t size)
{
    my_log("[INFO] Reallocating memory block at %p with new size %lu\n", ptr, size);
    LOG_ENTER_FUNCTION();
    mygetlist();
    // If ptr is NULL, equivalent to calling my_malloc(size)
    
    if (ptr == NULL)
    {
        LOG_EXIT_FUNCTION();
        return my_malloc(size);
    }

    // If size is 0, equivalent to calling my_free(ptr) and returning NULL
    if (size == 0)
    {
        my_free(ptr);
        LOG_EXIT_FUNCTION();
        return NULL;
    }

    // Retrieve the meta_struck associated with the given pointer
    meta_struck *chunck = get_chunck(ptr);

    // If the meta_struck is not found, return NULL
    if (chunck == NULL)
    {
        my_log("[ERROR] Chunck not found\n");
        my_log("search for ptr %p\n", ptr);
        mygetlist();
        LOG_EXIT_FUNCTION();
        return NULL;
    }

    // If the current size of the memory block is already greater or equal to the new size,
    // return the original pointer
    if (chunck->sz_size >= size)
    {
        LOG_EXIT_FUNCTION();
        return ptr;
    }

    // Allocate a new memory block with the new size
    
    // Get all the free place next
    size_t free_place = 0;
    meta_struck *tmp = chunck;
    while (tmp->p_next != NULL && tmp->p_next->is_free == 1)
    {
        free_place += tmp->p_next->sz_size;
        tmp = tmp->p_next;
    } 

    // If the free place is enough to allocate the new size
    if (free_place >= size - chunck->sz_size)
    {
        // Update the size of the current block
        chunck->sz_size = size;

        // Update the size of the next free block
        tmp->sz_size = free_place - (size - chunck->sz_size);
        chunck->p_next = tmp;

        // Return the pointer to the current block
        LOG_EXIT_FUNCTION();
        return ptr;
    }

    void *new_ptr = my_malloc(size);
    
    // If the allocation failed, return NULL
    if (new_ptr == NULL)
    {
        LOG_EXIT_FUNCTION();
        return NULL;
    }

    // Copy the contents of the original memory block to the new memory block
    memcpy(new_ptr, ptr, chunck->sz_size);

    // Free the original memory block
    my_free(ptr);

    LOG_EXIT_FUNCTION();

    // Return the pointer to the new memory block
    return new_ptr;
}

/**
 * Allocates memory for an array of elements and initializes them to zero.
 *
 * This function allocates memory for an array of `nmemb` elements, each of size `size`,
 * and initializes all the elements to zero. It is similar to the standard `calloc` function.
 *
 * @param nmemb The number of elements to allocate memory for.
 * @param size The size of each element in bytes.
 * @return A pointer to the allocated memory, or `NULL` if the allocation fails.
 */
void *my_calloc(size_t nmemb, size_t size)
{
    LOG_ENTER_FUNCTION();
    mygetlist();

    void *ptr = my_malloc(nmemb * size);
    if (ptr == NULL)
    {
        LOG_EXIT_FUNCTION();
        return NULL;
    }
    memset(ptr, 0, nmemb * size);
    LOG_EXIT_FUNCTION();
    return ptr;
}

/**
 * Prints information about the given metadata structure.
 *
 * @param ptr A pointer to the metadata structure.
 */
void my_whoami(meta_struck *meta)
{

    my_log("\t[DEBUG] Metadata details: \n\
            \t\t[*] ptr=%p, \n\
            \t\t[*] p_ptr_data=%p, \n\
            \t\t[*] sz_size=%lu, \n\
            \t\t[*] is_free=%d, \n\
            \t\t[*] p_next=%p\n",
            meta, meta->p_ptr_data, meta->sz_size, meta->is_free, meta->p_next);
}

/**
 * Verifies if all allocated memory blocks have been freed and logs any leaks.
 */
void verify_memory_leaks()
{
    LOG_ENTER_FUNCTION();
    meta_struck *tmp = ptr_metahead;
    bool leaks_found = false;
    while (tmp != NULL)
    {
        if (!tmp->is_free)
        {
            my_log("[ERROR][MEMORY LEAK] Unfreed block detected: \n");
            my_whoami(tmp);
            my_free(tmp->p_ptr_data);
            leaks_found = true;
        }
        tmp = tmp->p_next;
    }
    if (!leaks_found)
    {
        my_log("[INFO] No memory leaks detected.\n");
    }
    LOG_EXIT_FUNCTION();
    close_logging();
}

__attribute__((constructor)) void init()
{
    atexit(verify_memory_leaks);
}

/**
 * THIS FUNCTION IS USED FOR DEBUGGING PURPOSES ONLY.
 * Retrieves and logs the list of meta structures in the secure memory allocator.
 * This function iterates through the linked list of meta structures starting from `ptr_metahead`,
 * and logs information about each meta structure using the `my_whoami` function.
 * The list is terminated when `tmp` becomes NULL.
 * Finally, the function logs the address of `ptr_metatail`.
 */
void mygetlist()
{
    LOG_ENTER_FUNCTION();
    my_log("[############## metahead ##############]\n%p\n", ptr_metahead);
    meta_struck *tmp = ptr_metahead;
    while (tmp != NULL)
    {
        my_whoami(tmp);
        tmp = tmp->p_next;
    }
    my_log("[############## metatail ##############]\n%p\n", ptr_metatail);
    // my_whoami(ptr_metatail);
    LOG_EXIT_FUNCTION();
}



// Function to remap the metadata area
void *remap_metadata(size_t new_size)
{
    LOG_ENTER_FUNCTION();
    write(STDERR_FILENO, "Remapping metadata\n", 20);
    size_t old_size = sizeof(meta_struck) * METADATA_SIZE;
    void *new_ptr = mremap(ptr_metahead, old_size, new_size, MREMAP_MAYMOVE);
    if (new_ptr == MAP_FAILED)
    {
        my_log("[ERROR] mremap failed\n");
        LOG_EXIT_FUNCTION();
        return NULL;
    }
    ptr_metahead = new_ptr;
    LOG_EXIT_FUNCTION();
    return ptr_metahead;
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