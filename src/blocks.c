#include "blocks.h"

#include "heap.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static memory_block_t* head_block = NULL;
static memory_block_t* tail_block = NULL;

void view_blocks() {
    printf("—————————————————————————————\n");
    if (head_block == NULL) {
        printf("Hap has not initialized\n");
        return;
    }
    printf("head_block: %p\n", head_block);
    printf("tail_block: %p\n", tail_block);

    printf("Blocks:\n");
    for (const memory_block_t* it = head_block; it != NULL; it = it->next) {
        printf("%p:\t\t size: %lu,\t\t is_free: %d,\t\t next: %p\n", it, it->size, it->is_free, it->next);
    }

    printf("—————————————————————————————\n");
}

void* block_get_ptr_to_data(const memory_block_t* block) {
    return (void*)block + sizeof(memory_block_t);
}

// Only free blocks can be split
void* block_split(memory_block_t* parent_block, const size_t first_child_size) {
    if (parent_block == NULL) {
        return NULL;
    }
    if ((parent_block->size < sizeof(memory_block_t) + first_child_size)
        || !parent_block->is_free) {
        return NULL;
    }

    memory_block_t* first_child = parent_block;
    memory_block_t* second_child = block_get_ptr_to_data(parent_block) + first_child_size;

    second_child->size = parent_block->size - sizeof(memory_block_t) - first_child_size;
    second_child->is_free = true;
    second_child->next = parent_block->next;

    first_child->size = first_child_size;
    first_child->is_free = true;
    first_child->next = second_child;

    return second_child;
}

// Only free blocks can be merged
void* block_merge_with_next(memory_block_t* ptr) {
    if (ptr == NULL || !ptr->is_free) {
        return NULL;
    }
    if (ptr->next == NULL) {
        return ptr;
    }
    if (!ptr->next->is_free) {
        return NULL;
    }

    ptr->size += sizeof(memory_block_t) + ptr->next->size;
    ptr->next = ptr->next->next;

    return ptr;
}

memory_block_t* find_block(const void* ptr) {
    if (head_block == NULL) {
        return NULL;
    }

    for (memory_block_t* it = head_block; it != NULL; it = it->next) {
        if (block_get_ptr_to_data(it) == ptr) {
            return it;
        }
    }

    return NULL;
}

memory_block_t* get_parent_block(const memory_block_t* block) {
    if (block == NULL) {
        return NULL;
    }

    for (memory_block_t* it = head_block; it->next != NULL; it = it->next) {
        if (it->next == block) {
            return it;
        }
    }

    return NULL;
}

void list_init() {
    const void* heap_top = get_heap_top();
    const long heap_extended_size = extend_heap();

    head_block = (memory_block_t*)heap_top;
    tail_block = (memory_block_t*)heap_top;

    head_block->size = heap_extended_size - sizeof(memory_block_t);
    head_block->is_free = true;
    head_block->next = NULL;
}

void* block_add(const size_t size) {
    if (head_block == NULL) {
        list_init();
    }

    for (memory_block_t* it = head_block; it != NULL; it = it->next) {
        if (!it->is_free || (it->size < size + sizeof(memory_block_t))) {
            continue;
        }

        block_split(it, size);
        it->is_free = false;

        if (tail_block == it) {
            tail_block = it->next;
        }

        return block_get_ptr_to_data(it);
    }

    if (!tail_block->is_free) {
        const long heap_extended_size = extend_heap();

        memory_block_t* new_block = block_get_ptr_to_data(tail_block) + tail_block->size;

        new_block->size = heap_extended_size - sizeof(memory_block_t);
        new_block->is_free = true;
        new_block->next = NULL;

        tail_block->next = new_block;
        tail_block = new_block;
    }

    while (size > tail_block->size) {
        const long heap_extended_size = extend_heap();

        tail_block->size += heap_extended_size;
    }


    block_split(tail_block, size);

    memory_block_t* new_block = tail_block;
    new_block->is_free = false;

    tail_block = new_block->next;

    return block_get_ptr_to_data(new_block);
}

void block_free(const void* ptr) {
    if ((head_block == NULL) || (ptr == NULL)) {
        return;
    }

    memory_block_t* prev_block = NULL;

    for (memory_block_t* it = head_block; it != NULL; prev_block = it, it = it->next) {
        if (block_get_ptr_to_data(it) != ptr) {
            continue;
        }

        it->is_free = true;

        if ((prev_block != NULL) && prev_block->is_free) {
            it = block_merge_with_next(prev_block);
        }


        if ((it->next != NULL) && it->next->is_free) {
            if (it->next == tail_block) {
                tail_block = it;
            }
            block_merge_with_next(it);
        }

        return;
    }
}

bool is_in_blocks(const void* ptr) {
    if (head_block == NULL) {
        return false;
    }

    return find_block(ptr) != NULL;
}

size_t get_block_size(const void* ptr) {
    if (head_block == NULL) {
        return 0;
    }

    const memory_block_t* active_block = find_block(ptr);
    return (active_block != NULL && !active_block->is_free) ? active_block->size : 0;
}
