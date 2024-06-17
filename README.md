# Secure Memory Allocator

This project implements a secure memory allocator in C, developed as part of a school project. It provides functionalities similar to `malloc`, `free`, `realloc`, and `calloc` with added security measures.

## Overview

The secure memory allocator aims to demonstrate fundamental concepts of memory management and security in C programming. It implements secure memory allocation techniques to prevent common vulnerabilities such as buffer overflows and memory leaks.

## Features

- **Secure Memory Allocation**: Implements secure memory allocation techniques to prevent buffer overflows and other memory-related vulnerabilities.

- **Block Merging**: Optimizes memory usage by merging consecutive free memory blocks in the memory pool.

- **Memory Block Reallocation**: Supports reallocation of memory blocks using the `realloc` function, allowing resizing of memory blocks while preserving their contents.

- **Detection of Memory Leaks**: Tracks allocated memory blocks and ensures they are properly freed to prevent memory leaks.

- **Custom Logging**: Includes a custom logging mechanism for debugging purposes, allowing developers to print informative messages during runtime.

- **Dynamic Memory Management**: Provides custom implementations of memory allocation functions (`malloc`, `free`, `calloc`, `realloc`) for dynamic memory management.

## Usage

To use the secure memory allocator in your project, follow these steps:

1. Include the header file `my_secmalloc.private.h` in your source files.
2. Link the compiled object file `my_secmalloc.o` with your executable.

```c
#include "my_secmalloc.private.h"

int main() {
    // Allocate memory
    void* ptr = my_malloc(100);

    // Use the allocated memory

    // Free the allocated memory
    my_free(ptr);

    return 0;
}
```

## Building

To build the project, simply run the `make` command in your terminal:

```bash
make
```

This will compile the source files and generate the executable.

### Building Dynamic Library

To compile a dynamic library named `libmy_secmalloc.so`, use the following command:

```bash
make clean dynamic
```

This command will create the dynamic library in the `lib/` directory. The library exports the following symbols:

```bash
$ nm lib/libmy_secmalloc.so | grep " T " | grep -v my_ | cut -f3 -d' ' | sort
calloc
free
malloc
realloc
```

### Using LD_PRELOAD

You can use the `LD_PRELOAD` environment variable to force the use of your memory allocation functions for any program. For example:

```bash
LD_PRELOAD=./lib/libmy_secmalloc.so ./your_program
```

This will preload your secure memory allocator library (`libmy_secmalloc.so`) before executing `your_program`, ensuring that your memory allocation functions are used instead of the standard ones.

## LOG
You can use the 'MSM_OUTPUT' environment variable to set the file descriptor and 'LOG_LVL' environment variable to set the verbosity.
```bash
LOG_LVL=DEBUG MSM_OUTPUT=stdout LD_PRELOAD=./lib/libmy_secmalloc.so ./your_program
```


## Testing

To run the tests, execute the following command:

```bash
make test
```

This will compile the test suite and run the tests to ensure the proper functionality of the secure memory allocator.
