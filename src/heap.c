#include "heap.h"

#include <unistd.h>

static long get_page_size() {
    if (mem_page_size == 0) {
        mem_page_size = sysconf(_SC_PAGESIZE);
    }

    return mem_page_size;
}

const void* get_heap_top() {
    const void* heap_top = sbrk(0);
    if (heap_top == (void*)-1)
        return NULL;

    return heap_top;
}

long extend_heap() {
    const long extend_size = PAGES_AMOUNT_FOR_EXTEND * get_page_size();

    const void* old_heap_top = sbrk(extend_size);
    if (old_heap_top == (void*)-1)
        return 0;

    return extend_size;
}