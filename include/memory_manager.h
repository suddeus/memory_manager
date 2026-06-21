#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
void view_heap();
#endif

// Allocation <size> bytes in memory
// Returns pointer to allocated memory or NULL if memory has not allocated
void* s_malloc(size_t size);

// Free allocated memory
// If pointer is not match to any allocated pointer, nothing happens
void s_free(const void* ptr);

// Allocate zero-initialized memory for an array of `count` elements,
// each `size` bytes wide.
// Returns a pointer to the allocated memory, or NULL if allocation fails
// or if `count * size` overflows.
void* s_calloc(size_t count, size_t size);

// TODO
void* s_realloc(void* ptr, size_t size);

#ifdef __cplusplus
}
#endif