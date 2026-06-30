#pragma once

#include <stddef.h>
#include <stdbool.h>

/*
Memory blocks in the heap form a linked list.

Invariants:
    + `memory_block_t* head_block` points to first block and `memory_block_t* tail_block` points to last block
    + Two consecutive blocks must not both have is_free == true.
    + size stores only the size of the user-allocated memory.
*/

typedef struct memory_block {
    size_t size;
    bool is_free;
    struct memory_block* next;
} memory_block_t;

// view_blocks is available from outside only with DEBUG flag
void view_blocks();

void* block_add(size_t size);

void block_free(const void* ptr);

void* block_realloc(void* ptr, size_t size);
