//
// Created by whyiskra on 16.12.23.
//

#include <string.h>
#include "morphine/object/table.h"
#include "morphine/utils/overflow.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"
#include "morphine/params.h"

#define ROOT(tree) (&(tree)->root)
#define NIL_LEAF(tree) (&(tree)->nil_leaf)
#define FIRST(tree) ((tree)->root.left)

static inline ml_size sabs(ml_size a, ml_size b) {
    return a > b ? a - b : b - a;
}

static inline size_t hash2index(ml_hash hash, size_t size) {
    return (size_t) (hash % size);
}

// linked list of buckets

static inline struct bucket *insert_bucket(morphine_instance_t I, struct table *table) {
    overflow_add(table->hashmap.buckets.count, 1, MLIMIT_SIZE_MAX) {
        throwI_error(I, "table size limit has been exceeded");
    }

    struct bucket *bucket = allocI_uni(I, NULL, sizeof(struct bucket));
    bucket->ll.index = table->hashmap.buckets.count;
    table->hashmap.buckets.count++;

    {
        struct bucket **head = &table->hashmap.buckets.head;

        if ((*head) == NULL) {
            bucket->ll.prev = NULL;
            bucket->ll.next = NULL;

            table->hashmap.buckets.tail = bucket;
        } else {
            (*head)->ll.next = bucket;
            bucket->ll.prev = (*head);
            bucket->ll.next = NULL;
        }

        (*head) = bucket;
    }

    return bucket;
}

static inline void remove_bucket(morphine_instance_t I, struct table *table, struct bucket *bucket) {
    overflow_sub(table->hashmap.buckets.count, 1, 0) {
        throwI_error(I, "corrupted table buckets count");
    }

    {
        struct bucket *current = bucket->ll.next;
        while (current != NULL) {
            current->ll.index--;
            current = current->ll.next;
        }
    }

    if (table->hashmap.buckets.access == bucket) {
        if (bucket->ll.next != NULL) {
            table->hashmap.buckets.access = bucket->ll.next;
        } else {
            table->hashmap.buckets.access = bucket->ll.prev;
        }
    }

    if (bucket->ll.next != NULL) {
        bucket->ll.next->ll.prev = bucket->ll.prev;
    } else {
        table->hashmap.buckets.head = bucket->ll.prev;

        if (bucket->ll.prev == NULL) {
            table->hashmap.buckets.tail = NULL;
        }
    }

    if (bucket->ll.prev != NULL) {
        bucket->ll.prev->ll.next = bucket->ll.next;
    }

    table->hashmap.buckets.count--;
    allocI_free(I, bucket);
}

static inline struct bucket *get_bucket_by_index(struct hashmap *hashmap, ml_size index) {
    ml_size abs_access = hashmap->buckets.access == NULL ? MLIMIT_SIZE_MAX :
                         sabs(hashmap->buckets.access->ll.index, index);
    ml_size abs_head = hashmap->buckets.head == NULL ? MLIMIT_SIZE_MAX :
                       sabs(hashmap->buckets.head->ll.index, index);

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

static inline struct bucket *redblacktree_find(morphine_instance_t I, struct tree *tree, struct value key) {
    struct bucket *current = FIRST(tree);
    while (current != NIL_LEAF(tree)) {
        int cmp = valueI_compare(I, key, current->pair.key);

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

static inline bool redblacktree_insert_first(
    morphine_instance_t I,
    struct tree *tree,
    struct value key,
    struct bucket **current,
    struct bucket **parent
) {
    *current = FIRST(tree);
    *parent = ROOT(tree);

    while (*current != NIL_LEAF(tree)) {
        int cmp = valueI_compare(I, key, (*current)->pair.key);

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

static inline bool redblacktree_insert_second(
    morphine_instance_t I,
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

    int cmp = valueI_compare(I, pair.key, parent->pair.key);
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

static struct bucket *redblacktree_delete(morphine_instance_t I, struct tree *tree, struct value key) {
    struct bucket *bucket = redblacktree_find(I, tree, key);

    if (bucket == NULL) {
        return NULL;
    }

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

    return bucket;
}

static void redblacktree_init(struct tree *tree) {
    tree->nil_leaf = (struct bucket) {
        .left = NIL_LEAF(tree),
        .right = NIL_LEAF(tree),
        .parent = NIL_LEAF(tree),
        .color = BUCKET_COLOR_BLACK,
        .pair = valueI_pair(valueI_nil, valueI_nil),
    };

    tree->root = (struct bucket) {
        .left = NIL_LEAF(tree),
        .right = NIL_LEAF(tree),
        .parent = NIL_LEAF(tree),
        .color = BUCKET_COLOR_BLACK,
        .pair = valueI_pair(valueI_nil, valueI_nil),
    };
}

// table

static inline void resize(morphine_instance_t I, struct hashmap *hashmap) {
    bool need = hashmap->hashing.size > 0 &&
                ((hashmap->hashing.used * 10) / hashmap->hashing.size) < (MPARAM_TABLE_GROW_PERCENTAGE / 10);
    if (likely(need || hashmap->hashing.size >= MLIMIT_TABLE_TREES)) {
        return;
    }

    size_t new_size = 8;
    if (hashmap->hashing.size > 0) {
        new_size = hashmap->hashing.size + hashmap->hashing.size / 2;
    }

    hashmap->hashing.trees = allocI_vec(
        I, hashmap->hashing.trees, new_size, sizeof(struct tree)
    );

    hashmap->hashing.size = new_size;

    for (size_t i = 0; i < new_size; i++) {
        redblacktree_init(hashmap->hashing.trees + i);
    }

    struct bucket *current = hashmap->buckets.head;
    while (current != NULL) {
        ml_hash hash = valueI_hash(I, current->pair.key);
        size_t index = hash2index(hash, new_size);

        struct tree *tree = hashmap->hashing.trees + index;

        struct bucket *tree_current;
        struct bucket *tree_parent;

        if (redblacktree_insert_first(I, tree, current->pair.key, &tree_current, &tree_parent)) {
            throwI_error(I, "duplicate while resizing");
        } else {
            redblacktree_insert_second(
                I,
                tree,
                current->pair,
                current,
                tree_parent
            );
        }

        current = current->ll.prev;
    }
}

static inline struct bucket *get(morphine_instance_t I, struct hashmap *hashmap, struct value key) {
    if (hashmap->hashing.size == 0) {
        return NULL;
    }

    ml_hash hash = valueI_hash(I, key);
    size_t index = hash2index(hash, hashmap->hashing.size);

    struct tree *tree = hashmap->hashing.trees + index;
    return redblacktree_find(I, tree, key);
}

struct table *tableI_create(morphine_instance_t I) {
    struct table *result = allocI_uni(I, NULL, sizeof(struct table));
    (*result) = (struct table) {
        .metatable = NULL,

        .mode.fixed = false,
        .mode.mutable = true,
        .mode.accessible = true,
        .mode.metatable_locked = false,
        .mode.locked = false,

        .hashmap.buckets.access = NULL,
        .hashmap.buckets.head = NULL,
        .hashmap.buckets.tail = NULL,
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

    allocI_free(I, table->hashmap.hashing.trees);
    allocI_free(I, table);
}

void tableI_mode_fixed(morphine_instance_t I, struct table *table, bool is_fixed) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    if (table->mode.locked) {
        throwI_error(I, "table is locked");
    }

    table->mode.fixed = is_fixed;
}

void tableI_mode_mutable(morphine_instance_t I, struct table *table, bool is_mutable) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    if (table->mode.locked) {
        throwI_error(I, "table is locked");
    }

    table->mode.mutable = is_mutable;
}

void tableI_mode_accessible(morphine_instance_t I, struct table *table, bool is_accessible) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    if (table->mode.locked) {
        throwI_error(I, "table is locked");
    }

    table->mode.accessible = is_accessible;
}

void tableI_mode_lock_metatable(morphine_instance_t I, struct table *table) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    if (table->mode.locked) {
        throwI_error(I, "table is locked");
    }

    table->mode.metatable_locked = true;
}

void tableI_mode_lock(morphine_instance_t I, struct table *table) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    table->mode.locked = true;
}

ml_size tableI_size(morphine_instance_t I, struct table *table) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    return table->hashmap.buckets.count;
}

void tableI_set(morphine_instance_t I, struct table *table, struct value key, struct value value) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    if (!table->mode.mutable) {
        throwI_error(I, "table is immutable");
    }

    struct hashmap *hashmap = &table->hashmap;

    {
        gcI_safe_enter(I);
        gcI_safe(I, valueI_object(table));
        gcI_safe(I, key);
        gcI_safe(I, value);

        resize(I, hashmap);

        gcI_safe_exit(I);
    }

    gcI_barrier(I, table, key);
    gcI_barrier(I, table, value);

    ml_hash hash = valueI_hash(I, key);
    size_t index = hash2index(hash, hashmap->hashing.size);

    struct tree *tree = hashmap->hashing.trees + index;

    struct bucket *current;
    struct bucket *parent;

    if (unlikely(redblacktree_insert_first(I, tree, key, &current, &parent))) {
        current->pair.value = value;
        return;
    }

    if (table->mode.fixed) {
        throwI_error(I, "table is fixed");
    }

    current = insert_bucket(I, table);

    bool first = redblacktree_insert_second(
        I,
        tree,
        valueI_pair(key, value),
        current,
        parent
    );

    if (likely(first)) {
        hashmap->hashing.used++;
    }
}

bool tableI_has(morphine_instance_t I, struct table *table, struct value key) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    return get(I, &table->hashmap, key) != NULL;
}

struct value tableI_get(morphine_instance_t I, struct table *table, struct value key, bool *has) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    if (!table->mode.accessible) {
        throwI_error(I, "table is inaccessible");
    }

    struct hashmap *hashmap = &table->hashmap;

    struct bucket *found = get(I, hashmap, key);

    if (has != NULL) {
        (*has) = (found != NULL);
    }

    if (unlikely(found == NULL)) {
        return valueI_nil;
    } else {
        return found->pair.value;
    }
}

void tableI_idx_set(morphine_instance_t I, struct table *table, ml_size index, struct value value) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    if (!table->mode.mutable) {
        throwI_error(I, "table is immutable");
    }

    if (index >= table->hashmap.buckets.count) {
        throwI_error(I, "table index out of bounce");
    }

    struct bucket *current = get_bucket_by_index(&table->hashmap, index);
    if (current == NULL || current->ll.index != index) {
        throwI_error(I, "corrupted table buckets");
    }
    table->hashmap.buckets.access = current;

    gcI_barrier(I, table, value);
    current->pair.value = value;
}

struct pair tableI_idx_get(morphine_instance_t I, struct table *table, ml_size index, bool *has) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    if (!table->mode.accessible) {
        throwI_error(I, "table is inaccessible");
    }

    if (index >= table->hashmap.buckets.count) {
        if (has != NULL) {
            (*has) = false;
        }

        return valueI_pair(valueI_nil, valueI_nil);
    }

    struct bucket *current = get_bucket_by_index(&table->hashmap, index);
    if (current == NULL || current->ll.index != index) {
        throwI_error(I, "corrupted table buckets");
    }
    table->hashmap.buckets.access = current;

    if (has != NULL) {
        (*has) = true;
    }

    return current->pair;
}

struct value tableI_remove(morphine_instance_t I, struct table *table, struct value key, bool *has) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    if (!table->mode.mutable) {
        throwI_error(I, "table is immutable");
    }

    if (table->mode.fixed) {
        throwI_error(I, "table is fixed");
    }

    struct hashmap *hashmap = &table->hashmap;

    if (hashmap->hashing.size == 0) {
        goto notfound;
    }

    ml_hash hash = valueI_hash(I, key);
    size_t index = hash2index(hash, hashmap->hashing.size);

    struct tree *tree = hashmap->hashing.trees + index;
    struct bucket *bucket = redblacktree_delete(I, tree, key);
    if (bucket == NULL) {
        goto notfound;
    }

    struct value value = bucket->pair.value;
    remove_bucket(I, table, bucket);

    if (has != NULL) {
        *has = true;
    }

    return value;

notfound:
    if (has != NULL) {
        *has = false;
    }

    return valueI_nil;
}

void tableI_clear(morphine_instance_t I, struct table *table) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    if (!table->mode.mutable) {
        throwI_error(I, "table is immutable");
    }

    if (table->mode.fixed) {
        throwI_error(I, "table is fixed");
    }

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

        .buckets.head = NULL,
        .buckets.tail = NULL,
        .buckets.count = 0
    };
}

struct table *tableI_copy(morphine_instance_t I, struct table *table) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    if (!table->mode.accessible) {
        throwI_error(I, "table is inaccessible");
    }

    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(table));
    struct table *result = gcI_safe_obj(I, table, tableI_create(I));
    result->mode.mutable = table->mode.mutable;
    result->mode.fixed = table->mode.fixed;

    struct bucket *current = table->hashmap.buckets.tail;
    while (current != NULL) {
        tableI_set(I, result, current->pair.key, current->pair.value);

        current = current->ll.next;
    }

    gcI_safe_exit(I);

    return result;
}

struct value tableI_iterator_first(morphine_instance_t I, struct table *table, bool *has) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    if (!table->mode.accessible) {
        throwI_error(I, "table is inaccessible");
    }

    struct hashmap *hashmap = &table->hashmap;
    struct bucket *bucket = hashmap->buckets.tail;

    if (has != NULL) {
        (*has) = (bucket != NULL);
    }

    if (bucket == NULL) {
        return valueI_nil;
    }

    return bucket->pair.key;
}

struct pair tableI_iterator_next(morphine_instance_t I, struct table *table, struct value *key, bool *next) {
    if (table == NULL) {
        throwI_error(I, "table is null");
    }

    if (!table->mode.accessible) {
        throwI_error(I, "table is inaccessible");
    }

    if (key == NULL) {
        if (next != NULL) {
            (*next) = false;
        }

        return valueI_pair(valueI_nil, valueI_nil);
    }

    struct hashmap *hashmap = &table->hashmap;

    struct bucket *current = get(I, hashmap, *key);

    if (current == NULL) {
        if (next != NULL) {
            (*next) = false;
        }

        (*key) = valueI_nil;
        return valueI_pair(valueI_nil, valueI_nil);
    }

    struct bucket *next_bucket = current->ll.next;

    if (next != NULL) {
        (*next) = (next_bucket != NULL);
    }

    if (next_bucket == NULL) {
        (*key) = valueI_nil;
    } else {
        (*key) = next_bucket->pair.key;
    }

    return current->pair;
}

void tableI_packer_vectorize(struct table *table, struct packer_vectorize *V) {
    if (!table->mode.accessible) {
        packerI_vectorize_error(V, "table is inaccessible");
    }

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

void tableI_packer_write_info(struct table *table, struct packer_write *W) {
    if (!table->mode.accessible) {
        packerI_write_error(W, "table is inaccessible");
    }
}

void tableI_packer_write_data(struct table *table, struct packer_write *W) {
    if (!table->mode.accessible) {
        packerI_write_error(W, "table is inaccessible");
    }

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

    packerI_write_bool(W, table->mode.mutable);
    packerI_write_bool(W, table->mode.fixed);
    packerI_write_bool(W, table->mode.accessible);
    packerI_write_bool(W, table->mode.metatable_locked);
    packerI_write_bool(W, table->mode.locked);
}

struct table *tableI_packer_read_info(morphine_instance_t I, struct packer_read *R) {
    (void) R;
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

    tableI_mode_mutable(I, table, packerI_read_bool(R));
    tableI_mode_fixed(I, table, packerI_read_bool(R));
    tableI_mode_accessible(I, table, packerI_read_bool(R));

    if (packerI_read_bool(R)) {
        tableI_mode_lock_metatable(I, table);
    }

    if (packerI_read_bool(R)) {
        tableI_mode_lock(I, table);
    }
}
