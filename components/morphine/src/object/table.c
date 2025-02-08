//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/table.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"
#include "morphine/utils/overflow.h"
#include <string.h>

#define value_pair(k, v) ((struct pair) { .key = (k), .value = (v) })

#define ROOT(tree) (&(tree)->root)
#define NIL_LEAF(tree) (&(tree)->nil_leaf)
#define FIRST(tree) ((tree)->root.left)

static inline ml_size sabs(ml_size a, ml_size b) {
    return a > b ? a - b : b - a;
}

// linked list of buckets

static inline struct bucket *create_bucket(morphine_instance_t I, struct hashmap *hashmap) {
    mm_overflow_add(hashmap->buckets.count, 1) {
        throwI_error(I, "table size limit has been exceeded");
    }

    struct bucket *bucket = hashmap->buckets.uninit;
    if (bucket == NULL) {
        bucket = allocI_uni(I, NULL, sizeof(struct bucket));
        hashmap->buckets.uninit = bucket;
    }

    return bucket;
}

static inline void insert_bucket(morphine_instance_t I, struct hashmap *hashmap) {
    struct bucket *bucket = hashmap->buckets.uninit;
    if (bucket == NULL) {
        throwI_panic(I, "no uninited bucket");
    }
    hashmap->buckets.uninit = NULL;

    bucket->ll.index = hashmap->buckets.count;
    hashmap->buckets.count++;

    struct bucket **head = &hashmap->buckets.head;

    if ((*head) == NULL) {
        bucket->ll.prev = NULL;
        bucket->ll.next = NULL;

        hashmap->buckets.tail = bucket;
    } else {
        (*head)->ll.next = bucket;
        bucket->ll.prev = (*head);
        bucket->ll.next = NULL;
    }

    (*head) = bucket;
}

static inline void remove_bucket(morphine_instance_t I, struct hashmap *hashmap, struct bucket *bucket) {
    if (hashmap->buckets.count == 0) {
        throwI_panic(I, "corrupted table buckets count");
    }

    {
        struct bucket *current = bucket->ll.next;
        while (current != NULL) {
            current->ll.index--;
            current = current->ll.next;
        }
    }

    if (hashmap->buckets.access == bucket) {
        if (bucket->ll.next != NULL) {
            hashmap->buckets.access = bucket->ll.next;
        } else {
            hashmap->buckets.access = bucket->ll.prev;
        }
    }

    if (bucket->ll.next != NULL) {
        bucket->ll.next->ll.prev = bucket->ll.prev;
    }

    if (bucket->ll.prev != NULL) {
        bucket->ll.prev->ll.next = bucket->ll.next;
    }

    if (hashmap->buckets.head == bucket) {
        hashmap->buckets.head = bucket->ll.prev;
    }

    if (hashmap->buckets.tail == bucket) {
        hashmap->buckets.tail = bucket->ll.next;
    }

    hashmap->buckets.count--;
    allocI_free(I, bucket);
}

static inline struct bucket *get_bucket_by_index(struct hashmap *hashmap, ml_size index) {
    if (index >= hashmap->buckets.count) {
        return NULL;
    }

    ml_size abs_access =
        hashmap->buckets.access == NULL ? mm_typemax(ml_size) : sabs(hashmap->buckets.access->ll.index, index);
    ml_size abs_head =
        hashmap->buckets.head == NULL ? mm_typemax(ml_size) : sabs(hashmap->buckets.head->ll.index, index);

    struct bucket *current = NULL;
    if (hashmap->buckets.access != NULL && abs_access < abs_head && abs_access < index) {
        bool next = hashmap->buckets.access->ll.index < index;
        current = hashmap->buckets.access;
        for (ml_size i = 0; i < abs_access && current != NULL; i++) {
            if (next) {
                current = current->ll.next;
            } else {
                current = current->ll.prev;
            }
        }
    } else if (abs_head < abs_access && abs_head < index) {
        current = hashmap->buckets.head;
        for (ml_size i = 0; i < abs_head && current != NULL; i++) {
            current = current->ll.prev;
        }
    } else {
        current = hashmap->buckets.tail;
        for (ml_size i = 0; i < index && current != NULL; i++) {
            current = current->ll.next;
        }
    }

    return current;
}

// red-black tree

static inline bool redblacktree_is_empty(struct tree *tree) {
    return FIRST(tree) == NIL_LEAF(tree);
}

static inline struct bucket *redblacktree_find(struct tree *tree, struct value key) {
    struct bucket *current = FIRST(tree);
    while (current != NIL_LEAF(tree)) {
        int cmp = valueI_compare(key, current->pair.key);

        if (cmp == 0) {
            return current;
        }

        if (cmp < 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    return NULL;
}

static struct bucket *redblacktree_successor(struct tree *tree, struct bucket *bucket) {
    struct bucket *current = bucket->right;

    if (current != NIL_LEAF(tree)) {
        while (current->left != NIL_LEAF(tree)) {
            current = current->left;
        }
    } else {
        current = bucket->parent;
        while (bucket == current->right) {
            bucket = current;

            current = current->parent;
        }

        if (current == ROOT(tree)) {
            current = NULL;
        }
    }

    return current;
}

static inline void redblacktree_rotate_left(struct tree *tree, struct bucket *x) {
    struct bucket *y = x->right;

    x->right = y->left;
    if (x->right != NIL_LEAF(tree)) {
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

static inline void redblacktree_rotate_right(struct tree *tree, struct bucket *x) {
    struct bucket *y = x->left;

    x->left = y->right;
    if (x->left != NIL_LEAF(tree)) {
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

static void redblacktree_insert_repair(struct tree *tree, struct bucket *current) {
    struct bucket *uncle;

    do {
        if (current->parent == current->parent->parent->left) {
            uncle = current->parent->parent->right;
            if (uncle->color == BUCKET_COLOR_RED) {
                current->parent->color = BUCKET_COLOR_BLACK;
                uncle->color = BUCKET_COLOR_BLACK;

                current = current->parent->parent;
                current->color = BUCKET_COLOR_RED;
            } else {
                if (current == current->parent->right) {
                    current = current->parent;
                    redblacktree_rotate_left(tree, current);
                }

                current->parent->color = BUCKET_COLOR_BLACK;
                current->parent->parent->color = BUCKET_COLOR_RED;
                redblacktree_rotate_right(tree, current->parent->parent);
            }
        } else {
            uncle = current->parent->parent->left;
            if (uncle->color == BUCKET_COLOR_RED) {
                current->parent->color = BUCKET_COLOR_BLACK;
                uncle->color = BUCKET_COLOR_BLACK;

                current = current->parent->parent;
                current->color = BUCKET_COLOR_RED;
            } else {
                if (current == current->parent->left) {
                    current = current->parent;
                    redblacktree_rotate_right(tree, current);
                }

                current->parent->color = BUCKET_COLOR_BLACK;
                current->parent->parent->color = BUCKET_COLOR_RED;
                redblacktree_rotate_left(tree, current->parent->parent);
            }
        }
    } while (current->parent->color == BUCKET_COLOR_RED);
}

static inline bool redblacktree_insert_pre(
    struct tree *tree,
    struct value key,
    struct bucket **current,
    struct bucket **parent
) {
    *current = FIRST(tree);
    *parent = ROOT(tree);

    while (*current != NIL_LEAF(tree)) {
        int cmp = valueI_compare(key, (*current)->pair.key);

        if (cmp == 0) {
            return true;
        }

        (*parent) = (*current);

        if (cmp < 0) {
            (*current) = (*current)->left;
        } else {
            (*current) = (*current)->right;
        }
    }

    return false;
}

static inline bool redblacktree_insert_post(
    struct tree *tree,
    struct pair pair,
    struct bucket *current,
    struct bucket *parent
) {
    current->left = NIL_LEAF(tree);
    current->right = NIL_LEAF(tree);
    current->parent = parent;
    current->color = BUCKET_COLOR_RED;
    current->pair = pair;

    int cmp = valueI_compare(pair.key, parent->pair.key);
    if (parent == ROOT(tree) || cmp < 0) {
        parent->left = current;
    } else {
        parent->right = current;
    }

    if (current->parent->color == BUCKET_COLOR_RED) {
        redblacktree_insert_repair(tree, current);
    }

    FIRST(tree)->color = BUCKET_COLOR_BLACK;

    return parent == ROOT(tree);
}

static void delete_repair(struct tree *tree, struct bucket *current) {
    struct bucket *sibling;

    do {
        if (current == current->parent->left) {
            sibling = current->parent->right;

            if (sibling->color == BUCKET_COLOR_RED) {
                sibling->color = BUCKET_COLOR_BLACK;
                current->parent->color = BUCKET_COLOR_RED;

                redblacktree_rotate_left(tree, current->parent);
                sibling = current->parent->right;
            }

            if (sibling->right->color == BUCKET_COLOR_BLACK && sibling->left->color == BUCKET_COLOR_BLACK) {
                sibling->color = BUCKET_COLOR_RED;
                if (current->parent->color == BUCKET_COLOR_RED) {
                    current->parent->color = BUCKET_COLOR_BLACK;
                    break;
                } else {
                    current = current->parent;
                }
            } else {
                if (sibling->right->color == BUCKET_COLOR_BLACK) {
                    sibling->left->color = BUCKET_COLOR_BLACK;
                    sibling->color = BUCKET_COLOR_RED;

                    redblacktree_rotate_right(tree, sibling);
                    sibling = current->parent->right;
                }

                sibling->color = current->parent->color;
                current->parent->color = BUCKET_COLOR_BLACK;

                sibling->right->color = BUCKET_COLOR_BLACK;
                redblacktree_rotate_left(tree, current->parent);
                break;
            }
        } else {
            sibling = current->parent->left;

            if (sibling->color == BUCKET_COLOR_RED) {
                sibling->color = BUCKET_COLOR_BLACK;
                current->parent->color = BUCKET_COLOR_RED;

                redblacktree_rotate_right(tree, current->parent);
                sibling = current->parent->left;
            }

            if (sibling->right->color == BUCKET_COLOR_BLACK && sibling->left->color == BUCKET_COLOR_BLACK) {
                sibling->color = BUCKET_COLOR_RED;
                if (current->parent->color == BUCKET_COLOR_RED) {
                    current->parent->color = BUCKET_COLOR_BLACK;
                    break;
                } else {
                    current = current->parent;
                }
            } else {
                if (sibling->left->color == BUCKET_COLOR_BLACK) {
                    sibling->right->color = BUCKET_COLOR_BLACK;
                    sibling->color = BUCKET_COLOR_RED;

                    redblacktree_rotate_left(tree, sibling);
                    sibling = current->parent->left;
                }

                sibling->color = current->parent->color;
                current->parent->color = BUCKET_COLOR_BLACK;

                sibling->left->color = BUCKET_COLOR_BLACK;
                redblacktree_rotate_right(tree, current->parent);
                break;
            }
        }
    } while (current != FIRST(tree));
}

static inline void swap(struct tree *tree, struct bucket *bucket, struct bucket *remove) {
    if (bucket->parent->left == bucket) {
        bucket->parent->left = remove;
    }

    if (bucket->parent->right == bucket) {
        bucket->parent->right = remove;
    }

    if (bucket->left != NIL_LEAF(tree) && bucket->left->parent == bucket) {
        bucket->left->parent = remove;
    }

    if (bucket->right != NIL_LEAF(tree) && bucket->right->parent == bucket) {
        bucket->right->parent = remove;
    }

    remove->parent = bucket->parent;
    remove->left = bucket->left;
    remove->right = bucket->right;
    remove->color = bucket->color;
}

static void redblacktree_delete(struct tree *tree, struct bucket *bucket) {
    struct bucket *target;
    if (bucket->left == NIL_LEAF(tree) || bucket->right == NIL_LEAF(tree)) {
        target = bucket;
    } else {
        target = redblacktree_successor(tree, bucket);
    }

    struct bucket *child = target->left;
    if (child == NIL_LEAF(tree)) {
        child = target->right;
    }

    if (target->color == BUCKET_COLOR_BLACK) {
        if (child->color == BUCKET_COLOR_RED) {
            child->color = BUCKET_COLOR_BLACK;
        } else if (target != FIRST(tree)) {
            delete_repair(tree, target);
        }
    }

    if (child != NIL_LEAF(tree)) {
        child->parent = target->parent;
    }

    if (target == target->parent->left) {
        target->parent->left = child;
    } else {
        target->parent->right = child;
    }

    if (bucket != target) {
        swap(tree, bucket, target);
    }
}

static void redblacktree_init(struct tree *tree) {
    tree->nil_leaf = (struct bucket) {
        .left = NIL_LEAF(tree),
        .right = NIL_LEAF(tree),
        .parent = NIL_LEAF(tree),
        .color = BUCKET_COLOR_BLACK,
        .pair = value_pair(valueI_nil, valueI_nil),
    };

    tree->root = (struct bucket) {
        .left = NIL_LEAF(tree),
        .right = NIL_LEAF(tree),
        .parent = NIL_LEAF(tree),
        .color = BUCKET_COLOR_BLACK,
        .pair = value_pair(valueI_nil, valueI_nil),
    };
}

// table

static inline struct tree *hashmap_get_tree(struct hashmap *hashmap, struct value value) {
    size_t index = valueI_hash(value) % hashmap->hashing.size;
    return hashmap->hashing.trees + index;
}

static inline void hashmap_restruct_trees(struct hashmap *hashmap) {
    size_t size = hashmap->hashing.size;
    for (size_t i = 0; i < size; i++) {
        redblacktree_init(hashmap->hashing.trees + i);
    }

    struct bucket *current = hashmap->buckets.head;
    while (current != NULL) {
        struct tree *tree = hashmap_get_tree(hashmap, current->pair.key);

        struct bucket *tree_current;
        struct bucket *tree_parent;

        if (redblacktree_insert_pre(tree, current->pair.key, &tree_current, &tree_parent)) {
            tree_current->pair.value = current->pair.value;
        } else {
            redblacktree_insert_post(tree, current->pair, current, tree_parent);
        }

        current = current->ll.prev;
    }
}

static inline void hashmap_resize(morphine_instance_t I, struct hashmap *hashmap) {
    size_t old_size = hashmap->hashing.size;

    bool need = old_size == 0 || ((hashmap->hashing.used * 10) / old_size) >= (MPARAM_TABLE_GROW_PERCENT / 10);
    if (!need || old_size >= MPARAM_TABLE_MAX_TREES) {
        return;
    }

    size_t new_size = MPARAM_TABLE_INIT_TREES;
    if (old_size > 0) {
        new_size = mm_overflow_opc_add(old_size, old_size / 2, return);
    }

    hashmap->hashing.trees = allocI_vec(I, hashmap->hashing.trees, new_size, sizeof(struct tree));
    hashmap->hashing.size = new_size;
    hashmap_restruct_trees(hashmap);
}

static inline struct bucket *hashmap_get(struct hashmap *hashmap, struct value key, struct tree **tree) {
    if (hashmap->hashing.size == 0) {
        return NULL;
    }

    struct tree *found_tree = hashmap_get_tree(hashmap, key);
    if (tree != NULL) {
        *tree = found_tree;
    }

    return redblacktree_find(found_tree, key);
}

static inline struct pair hashmap_remove(morphine_instance_t I, struct hashmap *hashmap, struct value key, bool *has) {
    struct tree *tree = NULL;
    struct bucket *bucket = hashmap_get(hashmap, key, &tree);
    if (has != NULL) {
        *has = bucket != NULL;
    }

    if (bucket == NULL) {
        return value_pair(valueI_nil, valueI_nil);
    }

    redblacktree_delete(tree, bucket);
    if (redblacktree_is_empty(tree)) {
        hashmap->hashing.used = mm_overflow_opc_sub(hashmap->hashing.used, 1, throwI_panic(I, "broken table"));
    }

    struct pair pair = bucket->pair;
    remove_bucket(I, hashmap, bucket);
    return pair;
}

static inline struct bucket *hashmap_getidx(morphine_instance_t I, struct hashmap *hashmap, ml_size index) {
    if (index >= hashmap->buckets.count) {
        throwI_error(I, "table index out of bounce");
    }

    struct bucket *current = get_bucket_by_index(hashmap, index);
    if (current == NULL || current->ll.index != index) {
        throwI_error(I, "corrupted table buckets");
    }
    hashmap->buckets.access = current;

    return current;
}

// public

struct table *tableI_create(morphine_instance_t I) {
    struct table *result = allocI_uni(I, NULL, sizeof(struct table));
    (*result) = (struct table) {
        .metatable = NULL,

        .hashmap.buckets.access = NULL,
        .hashmap.buckets.head = NULL,
        .hashmap.buckets.tail = NULL,
        .hashmap.buckets.uninit = NULL,
        .hashmap.buckets.count = 0,

        .hashmap.hashing.trees = NULL,
        .hashmap.hashing.used = 0,
        .hashmap.hashing.size = 0,
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_TABLE);

    return result;
}

void tableI_free(morphine_instance_t I, struct table *table) {
    struct bucket *current = table->hashmap.buckets.head;
    while (current != NULL) {
        struct bucket *prev = current->ll.prev;
        allocI_free(I, current);
        current = prev;
    }

    if (table->hashmap.buckets.uninit != NULL) {
        allocI_free(I, table->hashmap.buckets.uninit);
    }

    allocI_free(I, table->hashmap.hashing.trees);
    allocI_free(I, table);
}

ml_size tableI_size(struct table *table) {
    return table->hashmap.buckets.count;
}

bool tableI_has(struct table *table, struct value key) {
    return hashmap_get(&table->hashmap, key, NULL) != NULL;
}

struct value tableI_get(struct table *table, struct value key, bool *has) {
    struct bucket *found = hashmap_get(&table->hashmap, key, NULL);

    if (has != NULL) {
        *has = found != NULL;
    }

    if (found == NULL) {
        return valueI_nil;
    } else {
        return found->pair.value;
    }
}

void tableI_set(morphine_instance_t I, struct table *table, struct value key, struct value value) {
    struct hashmap *hashmap = &table->hashmap;
    if (valueI_is_nil(value)) {
        hashmap_remove(I, hashmap, key, NULL);
    } else {
        gcI_safe_enter(I);

        gcI_safe(I, valueI_object(table));
        gcI_safe(I, key);
        gcI_safe(I, value);

        struct bucket *parent;
        struct bucket *current;

        hashmap_resize(I, hashmap);
        struct bucket *new_bucket = create_bucket(I, hashmap);
        struct tree *tree = hashmap_get_tree(hashmap, key);

        gcI_valbarrier(I, table, key);
        gcI_valbarrier(I, table, value);
        if (redblacktree_insert_pre(tree, key, &current, &parent)) {
            current->pair.value = value;
        } else {
            if (redblacktree_insert_post(tree, value_pair(key, value), new_bucket, parent)) {
                hashmap->hashing.used++;
            }

            insert_bucket(I, hashmap);
        }

        gcI_safe_exit(I);
    }
}

struct value tableI_remove(morphine_instance_t I, struct table *table, struct value key, bool *has) {
    return hashmap_remove(I, &table->hashmap, key, has).value;
}

struct pair tableI_idx_get(morphine_instance_t I, struct table *table, ml_size index) {
    return hashmap_getidx(I, &table->hashmap, index)->pair;
}

void tableI_idx_set(morphine_instance_t I, struct table *table, ml_size index, struct value value) {
    struct bucket *current = hashmap_getidx(I, &table->hashmap, index);
    if (valueI_is_nil(value)) {
        tableI_remove(I, table, current->pair.key, NULL);
    } else {
        current->pair.value = gcI_valbarrier(I, table, value);
    }
}

struct pair tableI_idx_remove(morphine_instance_t I, struct table *table, ml_size index) {
    struct bucket *current = hashmap_getidx(I, &table->hashmap, index);
    return hashmap_remove(I, &table->hashmap, current->pair.key, NULL);
}

struct pair tableI_first(struct table *table, bool *has) {
    struct bucket *bucket = table->hashmap.buckets.tail;

    if (has != NULL) {
        (*has) = (bucket != NULL);
    }

    if (bucket == NULL) {
        return value_pair(valueI_nil, valueI_nil);
    }

    return bucket->pair;
}

struct pair tableI_next(struct table *table, struct value key, bool *has) {
    struct bucket *current = hashmap_get(&table->hashmap, key, NULL);
    if (current == NULL) {
        if (has != NULL) {
            (*has) = false;
        }

        return value_pair(valueI_nil, valueI_nil);
    }

    struct bucket *next_bucket = current->ll.next;
    if (next_bucket == NULL) {
        if (has != NULL) {
            (*has) = false;
        }

        return value_pair(valueI_nil, valueI_nil);
    }

    if (has != NULL) {
        (*has) = true;
    }

    return next_bucket->pair;
}

void tableI_clear(morphine_instance_t I, struct table *table) {
    struct bucket *current = table->hashmap.buckets.head;
    while (current != NULL) {
        struct bucket *prev = current->ll.prev;
        allocI_free(I, current);
        current = prev;
    }

    allocI_free(I, table->hashmap.hashing.trees);

    table->hashmap = (struct hashmap) {
        .hashing.trees = NULL,
        .hashing.used = 0,
        .hashing.size = 0,

        .buckets.access = NULL,
        .buckets.head = NULL,
        .buckets.tail = NULL,
        .buckets.count = 0,
    };
}

struct table *tableI_copy(morphine_instance_t I, struct table *table) {
    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(table));

    struct table *result = gcI_safe_obj(I, table, tableI_create(I));

    struct bucket *current = table->hashmap.buckets.tail;
    while (current != NULL) {
        tableI_set(I, result, current->pair.key, current->pair.value);
        current = current->ll.next;
    }

    gcI_safe_exit(I);

    return result;
}

struct table *tableI_concat(morphine_instance_t I, struct table *a, struct table *b) {
    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(a));
    gcI_safe(I, valueI_object(b));

    struct table *result = gcI_safe_obj(I, table, tableI_create(I));

    for (ml_size i = 0; i < a->hashmap.buckets.count; i++) {
        struct pair pair = tableI_idx_get(I, a, i);
        tableI_set(I, result, pair.key, pair.value);
    }

    for (ml_size i = 0; i < b->hashmap.buckets.count; i++) {
        struct pair pair = tableI_idx_get(I, b, i);
        tableI_set(I, result, pair.key, pair.value);
    }

    gcI_safe_exit(I);

    return result;
}

void tableI_packer_vectorize(mattr_unused morphine_instance_t I, struct table *table, struct packer_vectorize *V) {
    struct bucket *current = table->hashmap.buckets.tail;
    while (current != NULL) {
        packerI_vectorize_append(V, current->pair.key);
        packerI_vectorize_append(V, current->pair.value);

        current = current->ll.next;
    }

    if (table->metatable != NULL) {
        packerI_vectorize_append(V, valueI_object(table->metatable));
    }
}

void tableI_packer_write_info(
    mattr_unused morphine_instance_t I,
    mattr_unused struct table *table,
    mattr_unused struct packer_write *W
) { }

void tableI_packer_write_data(mattr_unused morphine_instance_t I, struct table *table, struct packer_write *W) {
    packerI_write_ml_size(W, table->hashmap.buckets.count);

    struct bucket *current = table->hashmap.buckets.tail;
    while (current != NULL) {
        packerI_write_value(W, current->pair.key);
        packerI_write_value(W, current->pair.value);

        current = current->ll.next;
    }

    packerI_write_bool(W, table->metatable != NULL);
    if (table->metatable != NULL) {
        packerI_write_value(W, valueI_object(table->metatable));
    }
}

struct table *tableI_packer_read_info(morphine_instance_t I, mattr_unused struct packer_read *R) {
    return tableI_create(I);
}

void tableI_packer_read_data(morphine_instance_t I, struct table *table, struct packer_read *R) {
    ml_size size = packerI_read_ml_size(R);
    for (ml_size i = 0; i < size; i++) {
        gcI_safe_enter(I);
        struct value key = gcI_safe(I, packerI_read_value(R));
        struct value value = gcI_safe(I, packerI_read_value(R));
        tableI_set(I, table, key, value);
        gcI_safe_exit(I);
    }

    bool has_metatable = packerI_read_bool(R);
    if (has_metatable) {
        gcI_safe_enter(I);
        struct value metatable = gcI_safe(I, packerI_read_value(R));
        metatableI_set(I, valueI_object(table), valueI_as_table_or_error(I, metatable));
        gcI_safe_exit(I);
    }
}
