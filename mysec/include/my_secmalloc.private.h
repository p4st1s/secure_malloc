#ifndef _SECMALLOC_PRIVATE_H
#define _SECMALLOC_PRIVATE_H

#include "my_secmalloc.h"
#include <stdbool.h>


typedef struct meta_struck {
    struct meta_struck *p_next;
    // struct meta_struck *p_prev;
    size_t sz_size;
    void *p_ptr_data;
    int is_free;
    // int i64_canari;
}meta_struck, *StackMetadata;

void    mygetlist();
void    *init_metadata();
void    my_log(const char *fmt, ...);
void    *my_malloc(size_t size);
void    my_free(void *ptr);
void    my_merge(); 
void    *my_calloc(size_t nmemb, size_t size);
void    *my_realloc(void *ptr, size_t size);
void    my_whoami(meta_struck *ptr);
void    *alloc_data(size_t size);
meta_struck *find_empty_block(size_t size);


#endif
