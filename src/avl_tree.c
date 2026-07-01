#include "avl_tree.h"

#include <sys/mman.h>
#include <string.h>

const size_t AVL_NODE_HEADER_SIZE = sizeof(avl_node_t);

static avl_node_t* avl_root = NULL;

void* avl_get_ptr_to_data(const avl_node_t* node) {
    if (!node) {
        return NULL;
    }
    return (void*)node + AVL_NODE_HEADER_SIZE;
}

avl_node_t* find_node(const void* ptr) {
    avl_node_t* node = avl_root;

    while (node) {
        if (avl_get_ptr_to_data(node) == ptr) {
            return node;
        }
        if (ptr < avl_get_ptr_to_data(node)) {
            node = node->left;
        } else {
            node = node->right;
        }
    }

    return NULL;
}


int get_height(const avl_node_t* node) {
    return node ? node->height : 0;
}

void update_height(avl_node_t* node) {
    if (node) {
        node->height = 1 + (get_height(node->left) > get_height(node->right) ? get_height(node->left) : get_height(node->right));
    }
}

int get_balance_value(const avl_node_t* node) {
    return node ? get_height(node->left) - get_height(node->right) : 0;
}

avl_node_t* create_node(const size_t size) {
    avl_node_t* node = mmap(NULL,
        AVL_NODE_HEADER_SIZE + size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANON,
        -1, 0);

    if (!node) {
        return NULL;
    }

    node->size = size;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;

    return node;
}

avl_node_t* left_rotate(avl_node_t* node) {
    avl_node_t* node_r = node->right;
    avl_node_t* node_rl = node_r->left;

    node_r->left = node;
    node->right = node_rl;

    update_height(node);
    update_height(node_r);

    return node_r;
}

avl_node_t* right_rotate(avl_node_t* node) {
    avl_node_t* node_l = node->left;
    avl_node_t* node_lr = node_l->right;

    node_l->right = node;
    node->left = node_lr;

    update_height(node);
    update_height(node_l);

    return node_l;
}

avl_node_t* rebalance(avl_node_t* node) {
    if (node == NULL) {
        return NULL;
    }

    const int balance = get_balance_value(node);

    if (balance > 1) {
        if (get_balance_value(node->left) < 0) {
            node->left = left_rotate(node->left);
        }

        return right_rotate(node);
    }

    if (balance < -1) {
        if (get_balance_value(node->right) < 0) {
            node->right = right_rotate(node->right);
        }

        return left_rotate(node);
    }

    return node;
}

avl_node_t* avl_insert(avl_node_t* node, avl_node_t* new_node) {
    if (!node) {
        return new_node;
    }

    if (new_node < node) {
        node->left = avl_insert(node->left, new_node);
    } else if (new_node < node) {
        node->right = avl_insert(node->right, new_node);
    } else {
        return node;
    }

    update_height(node);
    node = rebalance(node);

    return node;
}

avl_node_t* detach_min(avl_node_t* node, avl_node_t** min_node) {
    if (!node) {
        *min_node = NULL;
        return NULL;
    }

    if (node->left == NULL) {
        avl_node_t* temp = node->right;

        *min_node = node;
        node->height = 1;
        node->left = NULL;
        node->right = NULL;

        return temp;
    }

    node->left = detach_min(node->left, min_node);

    return rebalance(node);
}

avl_node_t* remove_node(avl_node_t* node, avl_node_t* node_to_remove) {
    if (!node) {
        return NULL;
    }

    if (node_to_remove < node) {
        node->left = remove_node(node->left, node_to_remove);
        return rebalance(node);
    }
    if (node_to_remove > node) {
        node->right = remove_node(node->right, node_to_remove);
        return rebalance(node);
    }

    avl_node_t* left = node->left;
    avl_node_t* right = node->right;

    node->right = NULL;
    node->left = NULL;
    munmap(node, AVL_NODE_HEADER_SIZE + node->size);

    if (right == NULL) {
        return left;
    }

    avl_node_t* min_in_right = NULL;
    right = detach_min(right, &min_in_right);

    min_in_right->left = left;
    min_in_right->right = right;

    return min_in_right;
}


void* avl_new_node(const size_t size) {
    avl_node_t* new_node = create_node(size);

    avl_root = avl_insert(avl_root, new_node);

    return avl_get_ptr_to_data(new_node);
}

void avl_free(const void* ptr) {
    avl_node_t* node = find_node(ptr);
    if (node) {
        avl_root = remove_node(avl_root, node);
    }
}

bool is_in_avl(const void* ptr) {
    if (avl_root == NULL) {
        return false;
    }

    return find_node(ptr) != NULL;
}

size_t get_node_size(const void* ptr) {
    if (avl_root == NULL) {
        return 0;
    }

    const avl_node_t* node = find_node(ptr);
    return node ? node->size : 0;
}
