//
// Created by whyiskra on 3/16/24.
//

#include <allocator.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define max(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })

#define RB_ROOT(rbtree) (&(rbtree)->root)
#define RB_NIL(rbtree) (&(rbtree)->nil)
#define RB_FIRST(rbtree) ((rbtree)->root.left)

struct allocated {
    void *pointer;
    size_t size;
};

enum rbcolor {
    RED,
    BLACK
};

struct rbnode {
    struct allocated allocated;
    struct rbnode *left;
    struct rbnode *right;
    struct rbnode *parent;
    enum rbcolor color;
};

struct rbtree {
    struct rbnode root;
    struct rbnode nil;
};

static struct rbnode *find(struct rbtree *rbtree, void *pointer) {
    struct rbnode *current = RB_FIRST(rbtree);
    while (current != RB_NIL(rbtree)) {
        ptrdiff_t cmp = pointer - current->allocated.pointer;

        if (cmp == 0) {
            return current;
        } else if (cmp < 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    return NULL;
}

static struct rbnode *successor(struct rbtree *rbtree, struct rbnode *node) {
    struct rbnode *current = node->right;

    if (current != RB_NIL(rbtree)) {
        while (current->left != RB_NIL(rbtree)) {
            current = current->left;
        }
    } else {
        current = node->parent;
        while (node == current->right) {
            node = current;

            current = current->parent;
        }

        if (current == RB_ROOT(rbtree)) {
            current = NULL;
        }
    }

    return current;
}

static void rotate_left(struct rbtree *rbtree, struct rbnode *x) {
    struct rbnode *y = x->right;

    x->right = y->left;
    if (x->right != RB_NIL(rbtree)) {
        x->right->parent = x;
    }

    y->parent = x->parent;
    if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

static void rotate_right(struct rbtree *rbtree, struct rbnode *x) {
    struct rbnode *y = x->left;

    x->left = y->right;
    if (x->left != RB_NIL(rbtree)) {
        x->left->parent = x;
    }

    y->parent = x->parent;
    if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->right = x;
    x->parent = y;
}

static void insert_repair(struct rbtree *rbtree, struct rbnode *current) {
    struct rbnode *uncle;

    do {
        if (current->parent == current->parent->parent->left) {
            uncle = current->parent->parent->right;
            if (uncle->color == RED) {
                current->parent->color = BLACK;
                uncle->color = BLACK;

                current = current->parent->parent;
                current->color = RED;
            } else {
                if (current == current->parent->right) {
                    current = current->parent;
                    rotate_left(rbtree, current);
                }

                current->parent->color = BLACK;
                current->parent->parent->color = RED;
                rotate_right(rbtree, current->parent->parent);
            }
        } else {
            uncle = current->parent->parent->left;
            if (uncle->color == RED) {
                current->parent->color = BLACK;
                uncle->color = BLACK;

                current = current->parent->parent;
                current->color = RED;
            } else {
                if (current == current->parent->left) {
                    current = current->parent;
                    rotate_right(rbtree, current);
                }

                current->parent->color = BLACK;
                current->parent->parent->color = RED;
                rotate_left(rbtree, current->parent->parent);
            }
        }
    } while (current->parent->color == RED);
}

static bool insert(struct rbtree *rbtree, struct allocated allocated) {
    struct rbnode *current = RB_FIRST(rbtree);
    struct rbnode *parent = RB_ROOT(rbtree);

    while (current != RB_NIL(rbtree)) {
        ptrdiff_t cmp = allocated.pointer - current->allocated.pointer;

        if (cmp == 0) {
            return false;
        }

        parent = current;

        if (cmp < 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    current = malloc(sizeof(struct rbnode));

    if (current == NULL) {
        fprintf(stderr, "Allocator: out of memory\n");
        abort();
    }

    current->left = RB_NIL(rbtree);
    current->right = RB_NIL(rbtree);
    current->parent = parent;
    current->color = RED;
    current->allocated = allocated;

    ptrdiff_t cmp = allocated.pointer - parent->allocated.pointer;
    if (parent == RB_ROOT(rbtree) || cmp < 0) {
        parent->left = current;
    } else {
        parent->right = current;
    }

    if (current->parent->color == RED) {
        insert_repair(rbtree, current);
    }

    RB_FIRST(rbtree)->color = BLACK;

    return true;
}

static void delete_repair(struct rbtree *rbtree, struct rbnode *current) {
    struct rbnode *sibling;

    do {
        if (current == current->parent->left) {
            sibling = current->parent->right;

            if (sibling->color == RED) {
                sibling->color = BLACK;
                current->parent->color = RED;

                rotate_left(rbtree, current->parent);
                sibling = current->parent->right;
            }

            if (sibling->right->color == BLACK && sibling->left->color == BLACK) {
                sibling->color = RED;
                if (current->parent->color == RED) {
                    current->parent->color = BLACK;
                    break;
                } else {
                    current = current->parent;
                }
            } else {
                if (sibling->right->color == BLACK) {
                    sibling->left->color = BLACK;
                    sibling->color = RED;

                    rotate_right(rbtree, sibling);
                    sibling = current->parent->right;
                }

                sibling->color = current->parent->color;
                current->parent->color = BLACK;

                sibling->right->color = BLACK;
                rotate_left(rbtree, current->parent);
                break;
            }
        } else {
            sibling = current->parent->left;

            if (sibling->color == RED) {
                sibling->color = BLACK;
                current->parent->color = RED;

                rotate_right(rbtree, current->parent);
                sibling = current->parent->left;
            }

            if (sibling->right->color == BLACK && sibling->left->color == BLACK) {
                sibling->color = RED;
                if (current->parent->color == RED) {
                    current->parent->color = BLACK;
                    break;
                } else {
                    current = current->parent;
                }
            } else {
                if (sibling->left->color == BLACK) {
                    sibling->right->color = BLACK;
                    sibling->color = RED;

                    rotate_left(rbtree, sibling);
                    sibling = current->parent->left;
                }

                sibling->color = current->parent->color;
                current->parent->color = BLACK;

                sibling->left->color = BLACK;
                rotate_right(rbtree, current->parent);
                break;
            }
        }
    } while (current != RB_FIRST(rbtree));
}

static struct allocated delete(struct rbtree *rbtree, void *pointer) {
    struct rbnode *node = find(rbtree, pointer);

    if (node == NULL) {
        return (struct allocated) {
            .pointer = NULL,
            .size = 0
        };
    }

    struct allocated data = node->allocated;

    struct rbnode *target;
    if (node->left == RB_NIL(rbtree) || node->right == RB_NIL(rbtree)) {
        target = node;
    } else {
        target = successor(rbtree, node);
        node->allocated = target->allocated;
    }

    struct rbnode *child = (target->left == RB_NIL(rbtree)) ? target->right : target->left;

    if (target->color == BLACK) {
        if (child->color == RED) {
            child->color = BLACK;
        } else if (target != RB_FIRST(rbtree)) {
            delete_repair(rbtree, target);
        }
    }

    if (child != RB_NIL(rbtree)) {
        child->parent = target->parent;
    }

    if (target == target->parent->left) {
        target->parent->left = child;
    } else {
        target->parent->right = child;
    }

    free(target);
    return data;
}

void allocator_init(struct allocator *allocator, size_t max_bytes) {
    struct rbtree *tree = malloc(sizeof(struct rbtree));

    if (tree == NULL) {
        fprintf(stderr, "Allocator: out of memory\n");
        abort();
    }

    tree->nil.left = RB_NIL(tree);
    tree->nil.right = RB_NIL(tree);
    tree->nil.parent = RB_NIL(tree);
    tree->nil.color = BLACK;
    tree->nil.allocated.pointer = NULL;

    tree->root.left = RB_NIL(tree);
    tree->root.right = RB_NIL(tree);
    tree->root.parent = RB_NIL(tree);
    tree->root.color = BLACK;
    tree->root.allocated.pointer = NULL;

    *allocator = (struct allocator) {
        .max_bytes = max_bytes,
        .allocated_bytes = 0,
        .allocations_count = 0,
        .peak_allocated_bytes = 0,
        .tree = tree
    };
}

void allocator_destroy(struct allocator *allocator) {
    allocator_clear(allocator);
    free(allocator->tree);

    *allocator = (struct allocator) {
        .max_bytes = 0,
        .allocated_bytes = 0,
        .allocations_count = 0,
        .peak_allocated_bytes = 0,
        .tree = NULL
    };
}

void *allocator_alloc(struct allocator *allocator, size_t size) {
    if (size == 0) {
        fprintf(stderr, "Allocator: Attempt to allocate zero size\n");
        abort();
    }

    if (allocator->allocated_bytes + size >= allocator->max_bytes) {
        fprintf(stderr, "Allocator: overflow\n");
        return NULL;
    }

    void *pointer = malloc(size);

    if (pointer == NULL) {
        fprintf(stderr, "Allocator: out of memory\n");
        abort();
    }

    struct allocated allocated = {
        .pointer = pointer,
        .size = size,
    };

    if (!insert(allocator->tree, allocated)) {
        fprintf(stderr, "Allocator: recognize failed\n");
        abort();
    }

    allocator->allocated_bytes += size;
    allocator->allocations_count++;
    allocator->peak_allocated_bytes = max(allocator->peak_allocated_bytes, allocator->allocated_bytes);

    return pointer;
}

void *allocator_realloc(struct allocator *allocator, void *pointer, size_t size) {
    if (size == 0) {
        fprintf(stderr, "Allocator: Attempt to reallocate to zero size\n");
        abort();
    }

    if (pointer == NULL) {
        fprintf(stderr, "Allocator: Attempt to reallocate null\n");
        abort();
    }

    struct allocated current = delete(allocator->tree, pointer);

    if (current.pointer == NULL) {
        fprintf(stderr, "Allocator: %p not found\n", pointer);
        abort();
    }

    if (allocator->allocated_bytes + size - current.size >= allocator->max_bytes) {
        fprintf(stderr, "Allocator: overflow\n");
        return NULL;
    }

    current.pointer = realloc(pointer, size);

    if (current.pointer == NULL) {
        fprintf(stderr, "Allocator: out of memory\n");
        abort();
    }

    allocator->allocated_bytes += size - current.size;

    current.size = size;

    insert(allocator->tree, current);

    allocator->allocations_count++;
    allocator->peak_allocated_bytes = max(allocator->peak_allocated_bytes, allocator->allocated_bytes);

    return current.pointer;
}

void allocator_free(struct allocator *allocator, void *pointer) {
    if (pointer == NULL) {
        fprintf(stderr, "Allocator: Attempt to free null\n");
        abort();
    }

    struct allocated found = delete(allocator->tree, pointer);

    if (found.pointer == NULL) {
        fprintf(stderr, "Allocator: Attempt to free unallocated pointer %p\n", pointer);
        abort();
    }

    free(found.pointer);

    allocator->allocated_bytes -= found.size;
}

static void clear(struct rbtree *rbtree, struct rbnode *node) {
    if (node == RB_NIL(rbtree)) {
        return;
    }

    clear(rbtree, node->left);
    clear(rbtree, node->right);

    free(node->allocated.pointer);
    free(node);
}

void allocator_clear(struct allocator *allocator) {
    struct rbtree *tree = allocator->tree;

    clear(tree, RB_FIRST(tree));

    tree->root.left = RB_NIL(tree);
    tree->root.right = RB_NIL(tree);
    tree->root.parent = RB_NIL(tree);
    tree->root.color = BLACK;
    tree->root.allocated.pointer = NULL;

    allocator->allocated_bytes = 0;
}
