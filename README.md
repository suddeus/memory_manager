# Memory Manager

Custom memory allocation library written in C

It is intended as an educational project for learning how dynamic memory allocation works internally


## Features
+ Allocations:
  + Small blocks allocates in the heap area
  + Large blocks allocates used mmap 
+ Reuses freed blocks when possible
+ Provides calloc-style zero-initialized allocation
+ Provides realloc-style resizing with data preservation
+ Includes unit tests based on GoogleTest


## Implementation overview

The allocator uses two allocation strategies:
+ Small allocations are managed through a linked list of memory blocks. The allocator splits free blocks when allocating memory and merges neighboring free blocks after freeing memory
+ Large allocations are handled separately with mmap. These allocations are tracked in an AVL tree, which allows the allocator to find and release large allocated blocks

Threshold for large allocations is 100 KB


## Public API

Include the public header:
```c
#include <memory_manager.h>
```

### `void* s_malloc(size_t size)`

Allocates size bytes of memory

Returns a pointer to the allocated memory, or `NULL` if allocation fails

### `void s_free(const void* ptr)`

Frees memory previously allocated by the allocator

If ptr is `NULL` or does not match an allocated block, the function does nothing

### `void* s_calloc(size_t count, size_t size)`

Allocates memory for an array of count elements, each size bytes wide

The allocated memory is initialized to zero

Returns `NULL` if allocation fails or if `count*size` overflows

### `void* s_realloc(void* ptr, size_t size)`

Changes the size of an allocated memory block

* If ptr is `NULL`, behaves like `s_malloc(size)`
* If size is 0, frees ptr and returns `NULL`
* Preserves existing data up to the smaller of the old and new sizes
* Returns `NULL` if allocation fails or if ptr is not managed by this allocator


## Build

Requirements:
* CMake 3.28 or newer
* C compiler with C11 support
* C++ compiler for tests

Build the project:
```shell
cmake -S . -B build
cmake --build build
```

This builds the shared library:
```
libmemory_manager.so
```


## Run tests

```shell
ctest --test-dir build
```
