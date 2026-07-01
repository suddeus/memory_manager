#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct Node {
    size_t size;
    int height;
    struct Node* left;
    struct Node* right;
} avl_node_t;


void* avl_new_node(size_t size);

void avl_free(const void* ptr);

bool is_in_avl(const void* ptr);

size_t get_node_size(const void* ptr);
