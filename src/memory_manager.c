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
    if (is_in_blocks(ptr)) {
        block_free(ptr);
    }
    if (is_in_avl(ptr)) {
        avl_free(ptr);
    }
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

    const bool was_in_avl = is_in_avl(ptr);
    const bool was_in_blocks = is_in_blocks(ptr);

    if (!was_in_blocks && !was_in_avl) {
        return NULL;
    }

    const size_t old_size = was_in_avl ? get_node_size(ptr) : get_block_size(ptr);

    if (old_size == size) {
        return ptr;
    }

    void* mem = s_malloc(size);

    memcpy(mem, ptr, old_size < size ? old_size : size);

    s_free(ptr);

    return mem;
}
