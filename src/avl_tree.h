#pragma once

#include <stddef.h>

typedef struct Node {
    size_t size;
    int height;
    struct Node* left;
    struct Node* right;
} avl_node_t;


void* avl_new_node(size_t size);

void avl_free(const void* ptr);

void* avl_realloc(void* ptr, size_t size);