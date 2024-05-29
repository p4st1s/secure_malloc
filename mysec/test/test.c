#include <criterion/criterion.h>
#include <stdio.h>
#include <string.h>
#include "my_secmalloc.private.h"
#include "my_secmalloc.h"
#include <sys/mman.h>

Test(mmap, simple) {
    void *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    cr_expect(ptr != NULL);
    int res = munmap(ptr, 4096);
    cr_expect(res == 0);
}

Test(my_malloc, simple) {
    void *ptr = my_malloc(4096);
    cr_expect(ptr != NULL);
    int res = munmap(ptr, 4096);
    cr_expect(res == 0);
}

Test(init_meta_data, simple2) {
    void *ptr = init_meta_data();
    cr_expect(ptr != NULL);    
}

Test(my_malloc, simple2) {
    void *ptr = my_malloc(4096);
    cr_expect(ptr != NULL);
    strncpy(ptr, "Hello", 6);
    cr_expect(strcmp(ptr, "Hello") == 0);
    int res = munmap(ptr, 4096);
    cr_expect(res == 0);   
}
