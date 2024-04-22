//
// Created by whyiskra on 16.12.23.
//

#include <string.h>
#include "morphine/object/table.h"
#include "morphine/object/string.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"

#define ROOT(tree) (&(tree)->root)
#define NIL_LEAF(tree) (&(tree)->nil_leaf)
#define FIRST(tree) ((tree)->root.left)

#define COMPARE_NUM(a, b) ((a) == (b) ? 0 : ((a) > (b) ? 1 : -1))

// compare & hash

static uint64_t hashcode(morphine_instance_t I, struct value value) {
    struct string *str = valueI_safe_as_string(value, NULL);
    if (str != NULL) {
        return stringI_hash(I, str);
    }

    switch (value.type) {
        case VALUE_TYPE_NIL:
            return (uint64_t) valueI_as_nil(value);
        case VALUE_TYPE_INTEGER:
            return (uint64_t) valueI_as_integer(value);
        case VALUE_TYPE_DECIMAL:
            return (uint64_t) valueI_as_decimal(value);
        case VALUE_TYPE_BOOLEAN:
            return (uint64_t) valueI_as_boolean(value);
        case VALUE_TYPE_RAW:
            return (uint64_t) valueI_as_raw(value);
        case VALUE_TYPE_USERDATA:
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_TABLE:
        case VALUE_TYPE_VECTOR:
        case VALUE_TYPE_CLOSURE:
        case VALUE_TYPE_COROUTINE:
        case VALUE_TYPE_FUNCTION:
        case VALUE_TYPE_NATIVE:
        case VALUE_TYPE_ITERATOR:
        case VALUE_TYPE_REFERENCE:
            return (uint64_t) valueI_as_object(value);
    }

    throwI_panic(I, "Unknown value type");
}

static inline int compare(morphine_instance_t I, struct value a, struct value b) {
    if (likely(a.type != b.type)) {
        return COMPARE_NUM(a.type, b.type);
    }

    switch (a.type) {
        case VALUE_TYPE_NIL:
            return 0;
        case VALUE_TYPE_INTEGER:
            return COMPARE_NUM(a.integer, b.integer);
        case VALUE_TYPE_DECIMAL:
            return COMPARE_NUM(a.decimal, b.decimal);
        case VALUE_TYPE_BOOLEAN:
            return COMPARE_NUM(a.boolean, b.boolean);
        case VALUE_TYPE_RAW:
            return COMPARE_NUM(a.raw, b.raw);
        case VALUE_TYPE_STRING: {
            struct string *str_a = valueI_as_string(a);
            struct string *str_b = valueI_as_string(b);
            return strcmp(str_a->chars, str_b->chars);
        }
        case VALUE_TYPE_USERDATA:
        case VALUE_TYPE_TABLE:
        case VALUE_TYPE_VECTOR:
        case VALUE_TYPE_CLOSURE:
        case VALUE_TYPE_COROUTINE:
        case VALUE_TYPE_FUNCTION:
        case VALUE_TYPE_NATIVE:
        case VALUE_TYPE_REFERENCE:
        case VALUE_TYPE_ITERATOR:
            return COMPARE_NUM(a.object.header, b.object.header);
    }

    throwI_panic(I, "Unsupported type");
}

// linked list of buckets

static inline void insert_bucket(struct table *table, struct bucket *bucket) {
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

static inline void remove_bucket(struct table *table, struct bucket *bucket) {
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
}

// red-black tree

static inline struct bucket *redblacktree_find(morphine_instance_t I, struct tree *tree, struct value key) {
    struct bucket *current = FIRST(tree);
    while (current != NIL_LEAF(tree)) {
        int cmp = compare(I, key, current->pair.key);

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
        int cmp = compare(I, key, (*current)->pair.key);

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

    int cmp = compare(I, pair.key, parent->pair.key);
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

static void resize(morphine_instance_t I, struct hashmap *hashmap) {
    bool need = hashmap->hashing.size > 0 &&
                ((hashmap->hashing.used * 10) / hashmap->hashing.size) < (TABLE_GROW_PERCENTAGE / 10);
    if (likely(need || hashmap->hashing.size >= 131070)) {
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
        uint64_t hash = hashcode(I, current->pair.key);
        size_t index = hash % new_size;

        struct tree *tree = hashmap->hashing.trees + index;

        struct bucket *tree_current;
        struct bucket *tree_parent;

        if (redblacktree_insert_first(I, tree, current->pair.key, &tree_current, &tree_parent)) {
            throwI_error(I, "Duplicate while resizing");
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

    uint64_t hash = hashcode(I, key);
    size_t index = hash % hashmap->hashing.size;

    struct tree *tree = hashmap->hashing.trees + index;
    return redblacktree_find(I, tree, key);
}

struct table *tableI_create(morphine_instance_t I) {
    struct table *result = allocI_uni(I, NULL, sizeof(struct table));

    (*result) = (struct table) {
        .metatable = NULL,

        .mode.fixed = false,
        .mode.mutable = true,
        .mode.locked = false,

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
        throwI_error(I, "Table is null");
    }

    if (table->mode.locked) {
        throwI_error(I, "Table is locked");
    }

    table->mode.fixed = is_fixed;
}

void tableI_mode_mutable(morphine_instance_t I, struct table *table, bool is_mutable) {
    if (table == NULL) {
        throwI_error(I, "Table is null");
    }

    if (table->mode.locked) {
        throwI_error(I, "Table is locked");
    }

    table->mode.mutable = is_mutable;
}

void tableI_mode_lock(morphine_instance_t I, struct table *table) {
    if (table == NULL) {
        throwI_error(I, "Table is null");
    }

    table->mode.locked = true;
}

ml_size tableI_size(morphine_instance_t I, struct table *table) {
    if (table == NULL) {
        throwI_error(I, "Table is null");
    }

    return table->hashmap.buckets.count;
}

void tableI_set(morphine_instance_t I, struct table *table, struct value key, struct value value) {
    if (table == NULL) {
        throwI_error(I, "Table is null");
    }

    if (!table->mode.mutable) {
        throwI_error(I, "Table is immutable");
    }

    struct hashmap *hashmap = &table->hashmap;

    {
        size_t rollback = gcI_safe_value(I, key);
        gcI_safe_value(I, value);

        resize(I, hashmap);

        gcI_reset_safe(I, rollback);
    }

    gcI_barrier(I, table, key);
    gcI_barrier(I, table, value);

    uint64_t hash = hashcode(I, key);
    size_t index = hash % hashmap->hashing.size;

    struct tree *tree = hashmap->hashing.trees + index;

    struct bucket *current;
    struct bucket *parent;

    if (unlikely(redblacktree_insert_first(I, tree, key, &current, &parent))) {
        current->pair.value = value;
        return;
    }

    if (table->mode.fixed) {
        throwI_error(I, "Table is fixed");
    }

    if (unlikely(hashmap->buckets.count > MLIMIT_SIZE_MAX - 1)) {
        throwI_error(I, "Table size too big");
    }

    current = allocI_uni(I, NULL, sizeof(struct bucket));
    insert_bucket(table, current);
    hashmap->buckets.count++;

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

struct value tableI_get(morphine_instance_t I, struct table *table, struct value key, bool *has) {
    if (table == NULL) {
        throwI_error(I, "Table is null");
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

bool tableI_remove(morphine_instance_t I, struct table *table, struct value key) {
    if (table == NULL) {
        throwI_error(I, "Table is null");
    }

    if (table->mode.fixed) {
        throwI_error(I, "Table is fixed");
    }

    if (!table->mode.mutable) {
        throwI_error(I, "Table is immutable");
    }

    struct hashmap *hashmap = &table->hashmap;

    if (hashmap->hashing.size == 0) {
        return false;
    }

    uint64_t hash = hashcode(I, key);
    size_t index = hash % hashmap->hashing.size;

    struct tree *tree = hashmap->hashing.trees + index;
    struct bucket *bucket = redblacktree_delete(I, tree, key);
    if (bucket == NULL) {
        return false;
    }

    remove_bucket(table, bucket);
    allocI_free(I, bucket);
    hashmap->buckets.count--;

    return true;
}

void tableI_clear(morphine_instance_t I, struct table *table) {
    if (table == NULL) {
        throwI_error(I, "Table is null");
    }

    if (table->mode.fixed) {
        throwI_error(I, "Table is fixed");
    }

    if (!table->mode.mutable) {
        throwI_error(I, "Table is immutable");
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
        throwI_error(I, "Table is null");
    }

    struct table *result = tableI_create(I);
    result->mode.mutable = table->mode.mutable;
    result->mode.fixed = table->mode.fixed;

    size_t rollback = gcI_safe_obj(I, objectI_cast(result));

    struct bucket *current = table->hashmap.buckets.tail;
    while (current != NULL) {
        tableI_set(I, result, current->pair.key, current->pair.value);

        current = current->ll.next;
    }

    gcI_reset_safe(I, rollback);

    return result;
}

struct value tableI_first(morphine_instance_t I, struct table *table, bool *has) {
    if (table == NULL) {
        throwI_error(I, "Table is null");
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

struct pair tableI_next(morphine_instance_t I, struct table *table, struct value *key, bool *next) {
    if (table == NULL) {
        throwI_error(I, "Table is null");
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
