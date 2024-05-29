#define _GNU_SOURCE
#include "my_secmalloc.private.h"
#include <stdio.h>
#include <alloca.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

// general variable

bool first_malloc = true;
static meta_struck *st_metahead = NULL;
static void *ptr_datahead = NULL;



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

/// @brief function that will allocate data.
/// @param size
/// @return pointer to data
void *my_malloc(size_t size)
{
    void *ptr;
    void *ptr_data;
    if (first_malloc)
    {
        first_malloc = false;
        my_log("First malloc\n");
        ptr = init_meta_data();
        if (ptr == NULL)
        {
            my_log("Error in init_meta_data\n");
            return NULL;
        }
        my_log("allocated size = %d\n", size);
        ptr_data = mmap(ptr+sizeof(meta_struck)+1000, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        meta_struck *meta = (meta_struck *)ptr;
        st_metahead = meta;
        ptr_datahead = ptr_data;
        meta->p_next = NULL;
        meta->sz_size = 1000;
        meta->p_ptr_data = ptr_data;
        meta->is_free = 0;
        // log meta structure
        my_log("meta->p_next = %p\n", meta->p_next);
        my_log("meta->sz_size = %ld\n", meta->sz_size);
        my_log("meta->p_ptr_data = %p\n", meta->p_ptr_data);
        my_log("meta->is_free = %d\n", meta->is_free);
        // log des deux pointeurs
        my_log("ptr = %p\n", ptr);
        my_log("ptr_data = %p\n", ptr_data);
        my_log("st_metahead = %p\n", st_metahead);
        my_log("ptr_datahead = %p\n", ptr_datahead);
    }
    // use mmap to allocate memory
    return ptr_data;
}

void *init_meta_data()
{
    // allocate memory for metadata
    size_t meta_size = sizeof(meta_struck) * 1000;
    void *ptr = mmap(NULL, meta_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == NULL)
    {
        my_log("Error in mmap in init_meta_data\n");
        return NULL;
    }
    return ptr;
}

/// @brief free a pointer
/// @param ptr
void my_free(void *ptr)
{
    (void)ptr;
}

/// @brief
/// @param nmemb
/// @param size
/// @return
void *my_calloc(size_t nmemb, size_t size)
{
    (void)nmemb;
    (void)size;
    return NULL;
}

/// @brief function that realloc a data zone
/// @param ptr
/// @param size
/// @return
void *my_realloc(void *ptr, size_t size)
{
    (void)ptr;
    (void)size;
    return NULL;
}

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
