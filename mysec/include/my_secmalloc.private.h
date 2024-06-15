#ifndef _SECMALLOC_PRIVATE_H
#define _SECMALLOC_PRIVATE_H

#include "my_secmalloc.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct meta_struck 
{
    struct meta_struck *p_next;
    size_t sz_size;
    void *p_ptr_data;
    int is_free;
    uint32_t canari;
}meta_struck, *StackMetadata;

// ################ Function #################
void *init_pool(void *addr, size_t size);

void *init_metadata_pool();
meta_struck *find_free_chunck(size_t size);
void *get_chunck(size_t size);
void *split_chunck(meta_struck *ptr, size_t size);
void *alloc_chunck(size_t size);


void my_free(void *ptr);
void my_merge();

//log function
void my_log_meta_chunck (meta_struck *ptr);
void my_log_meta_chunck_all();

//security function

void check_canari(meta_struck *ptr);
void check_all_canari();
void check_memory_leak();
void clean_memory();




// ################ Function demandé #################
void *my_malloc(size_t size);
void my_free(void *ptr);
void *my_calloc(size_t nmemb, size_t size);
void *my_realloc(void *ptr, size_t size);


#endif
