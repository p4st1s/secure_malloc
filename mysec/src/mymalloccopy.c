#define _GNU_SOURCE
#include "my_secmalloc.private.h"
#include <stdio.h>
#include <alloca.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#define LOG_ENTER_FUNCTION() my_log("[INFO] Entering function %s\n", __func__)
#define LOG_EXIT_FUNCTION() my_log("[INFO] Exiting function %s\n", __func__)

#define METADATA_SIZE 4096
// ################ GLobal variable #################

meta_struck *ptr_metahead = NULL;
meta_struck *ptr_metatail = NULL;

// general function to print without using printf
void my_log(const char *fmt, ...)
{
    va_list ap_list;
    char *buffer;
    va_start(ap_list, fmt);
    size_t size = vsnprintf(NULL, 0, fmt, ap_list);
    va_end(ap_list);
    buffer = alloca(size + 2);
    va_start(ap_list, fmt);
    vsnprintf(buffer, size + 2, fmt, ap_list);
    va_end(ap_list);
    write(2, buffer, size);
}

// inizialize metadata
// and malloc the first data block
void *init_metadata()
{
    LOG_ENTER_FUNCTION();
    size_t meta_size = sizeof(meta_struck) * METADATA_SIZE;
    void *ptr = mmap(NULL, meta_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == NULL)
    {
        LOG_EXIT_FUNCTION();
        return NULL;
    }

    void *ptr_data = mmap(ptr + sizeof(meta_struck) * METADATA_SIZE, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    meta_struck *ptr_meta = (meta_struck *)ptr;
    ptr_meta->p_ptr_data = ptr_data;
    ptr_meta->sz_size = 4096;
    ptr_meta->is_free = 1;
    ptr_meta->p_next = NULL;
    ptr_metahead = ptr_meta;
    ptr_metatail = ptr_meta;
    my_log("[DEBUG] ptr_meta\n");
    // my_whoami(ptr_meta);
    LOG_EXIT_FUNCTION();
    return ptr;
}

// find empty block
// return NULL if no empty block is found
// else return the metadata of the empty block
meta_struck *find_empty_block(size_t size)
{
    LOG_ENTER_FUNCTION();
    meta_struck *tmp = ptr_metahead;
    while (tmp != NULL)
    {
        if (tmp->is_free == 1 && tmp->sz_size >= size)
        {
            my_log("[DEBUG] tmp \n");
            // my_whoami(tmp);
            LOG_EXIT_FUNCTION();
            return tmp;
        }
        tmp = tmp->p_next;
    }
    LOG_EXIT_FUNCTION();
    return NULL;
}

// allocate metadata block
// return NULL if allocation failed
// else return the metadata block
// params a définir
// meta_struck *alloc_metadata(void *actual, size_t size, void *ptr_data, int is_free)
// {
//     LOG_ENTER_FUNCTION();

//     LOG_EXIT_FUNCTION();
//     return NULL;
// }

// allocate a data block
// return NULL if allocation failed
// else return the data block ptr
void *my_malloc(size_t size)
{
    LOG_ENTER_FUNCTION();
    if (size <= 0)
    {
        my_log("[ERROR] Size is less than 0\n");
        LOG_EXIT_FUNCTION();
        return NULL;
    }
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

void *alloc_data_notempty(size_t size)
{
    my_log("[INFO] No empty block found\n");
    void *ptr_data = mmap(ptr_metahead + sizeof(meta_struck) * METADATA_SIZE, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    meta_struck *actual_ptr = (meta_struck *)ptr_metatail + sizeof(meta_struck);
    meta_struck *empty = (meta_struck *)ptr_metatail + (sizeof(meta_struck) * 2);
    actual_ptr->p_ptr_data = ptr_data;
    actual_ptr->sz_size = size;
    actual_ptr->is_free = 0;
    actual_ptr->p_next = empty;

    ptr_metatail->p_next = actual_ptr;
    ptr_metatail = actual_ptr;

    empty->p_ptr_data = actual_ptr->p_ptr_data + size;
    empty->sz_size = 4096 - (size % 4096);
    empty->is_free = 1;
    empty->p_next = NULL;

    ptr_metatail->p_next = empty;
    ptr_metatail = empty;

    my_log("[DEBUG] actual_ptr\n");
    // my_whoami(actual_ptr);
    my_log("[DEBUG] empty\n");
    // my_whoami(empty);

    LOG_EXIT_FUNCTION();
    return actual_ptr->p_ptr_data;
}

void *alloc_data_empty(size_t size, meta_struck *ptr_free)
{
    LOG_ENTER_FUNCTION();
    meta_struck *empty = (meta_struck *)ptr_metatail + sizeof(meta_struck);
    empty->p_ptr_data = ptr_free->p_ptr_data + size;
    empty->sz_size = ptr_free->sz_size - size;
    empty->is_free = 1;
    empty->p_next = NULL;

    ptr_metatail->p_next = empty;
    ptr_metatail = empty;
    
    ptr_free->sz_size = size;
    ptr_free->is_free = 0;
    my_log("[DEBUG] ptr_free\n");
    // my_whoami(ptr_free);
    my_log("[DEBUG] empty\n");
    // my_whoami(empty);
    LOG_EXIT_FUNCTION();
    return ptr_free->p_ptr_data;
}

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

// free an allocated data block
void my_free(void *ptr)
{
    LOG_ENTER_FUNCTION();
    while (ptr_metahead != NULL)
    {
        if (ptr_metahead->p_ptr_data == ptr)
        {
            if (ptr_metahead->is_free == 1)
            {
                my_log("[ERROR] Block is already free\n");
                break;
            }
            ptr_metahead->is_free = 1;
            break;
        }
        ptr_metahead = ptr_metahead->p_next;
    }
    LOG_EXIT_FUNCTION();
}

// reallocate an allocated data block
// return NULL if reallocation failed
// else return the new data block ptr

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

void *my_realloc(void *ptr, size_t size)
{
    LOG_ENTER_FUNCTION();
    if (ptr == NULL)
    {
        LOG_EXIT_FUNCTION();
        return my_malloc(size);
    }
    if (size == 0)
    {
        my_free(ptr);
        LOG_EXIT_FUNCTION();
        return NULL;
    }

    meta_struck *chunck = get_chunck(ptr);
    if (chunck == NULL)
    {
        my_log("[ERROR] Chunck not found\n");
        LOG_EXIT_FUNCTION();
        return NULL;
    }
    if (chunck->sz_size >= size)
    {
        LOG_EXIT_FUNCTION();
        return ptr;
    }

    void *new_ptr = my_malloc(size);
    if (new_ptr == NULL)
    {
        LOG_EXIT_FUNCTION();
        return NULL;
    }
    memcpy(new_ptr, ptr, chunck->sz_size);
    my_free(ptr);
    LOG_EXIT_FUNCTION();
    return new_ptr;
}

// allocate a data block
// return NULL if allocation failed
// else return the data block ptr full 0
void *my_calloc(size_t nmemb, size_t size)
{
    LOG_ENTER_FUNCTION();
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

// print metadata information
void my_whoami(meta_struck *ptr)
{
    LOG_ENTER_FUNCTION();
    my_log("[INFO] ##########################\n");
    my_log("\t[*] metadata is at %p\n", ptr);
    my_log("\t[*] ptr_data is %p\n", ptr->p_ptr_data);
    my_log("\t[*] size is %ld\n", ptr->sz_size);
    my_log("\t[*] p->next is %p\n", ptr->p_next);
    my_log("\t[*] is_free is %d\n", ptr->is_free);
    my_log("[INFO] ##########################\n\n");

    LOG_EXIT_FUNCTION();
}

void mygetlist()
{
    LOG_ENTER_FUNCTION();
    meta_struck *tmp = ptr_metahead;
    while (tmp != NULL)
    {
        // my_whoami(tmp);
        tmp = tmp->p_next;
    }
    LOG_EXIT_FUNCTION();
}

////////////////////////////////////////////////
// main
// #define ARRAY_SIZE 100
// #define DATA_SIZE 100

// int main()
// {
//     // Array to store pointers to allocated memory blocks
//     int *arrays[ARRAY_SIZE];

//     // Allocate memory blocks
//     for (int i = 0; i < ARRAY_SIZE; ++i)
//     {
//         arrays[i] = (int *)my_malloc(DATA_SIZE * sizeof(int));
//         if (arrays[i] == NULL)
//         {
//             // Handle allocation failure
//             fprintf(stderr, "Memory allocation failed for array %d\n", i);
//             // Free all allocated blocks before exiting
//             for (int j = 0; j < i; ++j)
//             {
//                 my_free(arrays[j]);
//             }
//             return -1;
//         }
//     }

//     // Fill the allocated blocks with some data
//     for (int i = 0; i < ARRAY_SIZE; ++i)
//     {
//         int *array = arrays[i];
//         for (int j = 0; j < DATA_SIZE; ++j)
//         {
//             array[j] = i * DATA_SIZE + j; // Filling with sequential data
//         }
//     }

//     // Print the contents of each array
//     printf("Contents of each array:\n");
//     for (int i = 0; i < ARRAY_SIZE; ++i)
//     {
//         printf("Array %d: ", i);
//         int *array = arrays[i];
//         for (int j = 0; j < DATA_SIZE; ++j)
//         {
//             printf("%d ", array[j]);
//         }
//         printf("\n");
//     }

//     // Free memory blocks
//     for (int i = 0; i < ARRAY_SIZE; ++i)
//     {
//         my_free(arrays[i]);
//     }

//     return 0;
// }

// for dynamic library
#ifdef DYNAMIC
void *malloc(size_t size)
{
    return my_malloc(size);
}
void free(void *ptr)
{
    my_free(ptr);
}
void *calloc(size_t nmemb, size_t size)
{
    return my_calloc(nmemb, size);
}

void *realloc(void *ptr, size_t size)
{
    return my_realloc(ptr, size);
}

#endif