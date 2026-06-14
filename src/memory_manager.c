#include <memory_manager.h>

#include "blocks.h"

// Threshold above which memory is allocated using mmap
// (if the size of the allocation is less than HEAP_MMAP_THRESHOLD,
// memory is allocated on the heap)
static const size_t HEAP_MMAP_THRESHOLD = 100 * 1024;

#ifdef DEBUG
void view_heap() {
    view_blocks();
}
#endif

void* s_malloc(const size_t size) {
    void* mem = NULL;

    if (size < HEAP_MMAP_THRESHOLD) {
        mem = block_add(size);
    } else {
        // TODO: Implement mmap allocation
        mem = NULL;
    }

    return mem;
}

void s_free(const void* ptr) {
    block_free(ptr);
}

void* s_calloc(size_t count, size_t size) {
    return NULL;
}

void* s_realloc(void* ptr, size_t size) {
    return NULL;
}
