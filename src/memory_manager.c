#include <memory_manager.h>

#include "blocks.h"
#include "avl_tree.h"

#include <stdint.h>
#include <string.h>

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
        mem = avl_new_node(size);
    }

    return mem;
}

void s_free(const void* ptr) {
    block_free(ptr);
    avl_free(ptr);
}

void* s_calloc(const size_t count, const size_t size) {
    if (count != 0 && size > SIZE_MAX / count) {
        // size_t overflow
        return NULL;
    }
    const size_t total_size = size * count;
    void* mem = s_malloc(total_size);

    if (mem == NULL) {
        return NULL;
    }
    memset(mem, 0, total_size);

    return mem;
}

void* s_realloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return s_malloc(size);
    }
    if (size == 0) {
        s_free(ptr);
        return NULL;
    }

    void* mem = NULL;

    if (size < HEAP_MMAP_THRESHOLD) {
        mem = block_realloc(ptr, size);
    } else {
        mem = avl_realloc(ptr, size);
    }

    return mem;
}
