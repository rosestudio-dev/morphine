//
// Created by whyiskra on 16.12.23.
//

#include <memory.h>
#include "morphine/object/table.h"
#include "morphine/object/string.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"

static inline uint64_t hashcode(morphine_instance_t I, struct value value) {
    struct string *str = valueI_safe_as_string(value, NULL);
    if (str != NULL) {
        uint64_t h = 0;
        for (ml_size i = 0; i < str->size; i++) {
            h = 31 * h + (str->chars[i] & 0xff);
        }

        return h;
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

static inline void resize(morphine_instance_t I, struct hashmap *hashmap) {
    if (hashmap->hashing.used < hashmap->hashing.size || hashmap->hashing.size >= (SIZE_MAX / 2)) {
        return;
    }

    size_t new_size = 8;
    if (hashmap->hashing.size > 0) {
        new_size = hashmap->hashing.size + hashmap->hashing.size / 2;
    }

    hashmap->hashing.trees = allocI_vec(
        I, hashmap->hashing.trees, new_size, sizeof(struct bucket *)
    );

    hashmap->hashing.size = new_size;

    for (size_t i = 0; i < new_size; i++) {
        hashmap->hashing.trees[i] = NULL;
    }

    struct bucket *current = hashmap->buckets.head;
    while (current != NULL) {
        uint64_t hash = hashcode(I, current->pair.key);
        size_t index = hash % new_size;

        current->tree.prev = hashmap->hashing.trees[index];
        hashmap->hashing.trees[index] = current;

        current = current->ll.prev;
    }
}

static inline struct bucket *get(morphine_instance_t I, struct hashmap *hashmap, struct value key) {
    if (hashmap->hashing.size == 0) {
        return NULL;
    }

    uint64_t hash = hashcode(I, key);
    size_t index = hash % hashmap->hashing.size;

    struct bucket *buckets = hashmap->hashing.trees[index];
    if (buckets != NULL) {
        struct bucket *current = buckets;
        while (current != NULL) {
            if (valueI_equal(I, current->pair.key, key)) {
                return current;
            }

            current = current->tree.prev;
        }
    }

    return NULL;
}

struct table *tableI_create(morphine_instance_t I) {
    struct table *result = allocI_uni(I, NULL, sizeof(struct table));

    (*result) = (struct table) {
        .metatable = NULL,

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

    gcI_barrier(I, table, key);
    gcI_barrier(I, table, value);

    struct hashmap *hashmap = &table->hashmap;

    resize(I, hashmap);

    uint64_t hash = hashcode(I, key);
    size_t index = hash % hashmap->hashing.size;

    struct bucket *buckets = hashmap->hashing.trees[index];
    if (buckets != NULL) {
        struct bucket *current = buckets;
        while (current != NULL) {
            if (valueI_equal(I, current->pair.key, key)) {
                current->pair.value = value;
                return;
            }

            current = current->tree.prev;
        }
    }

    if (unlikely(hashmap->buckets.count + 1 > MLIMIT_SIZE_MAX)) {
        throwI_error(I, "Table size too big");
    }

    struct bucket *bucket = allocI_uni(I, NULL, sizeof(struct bucket));

    (*bucket) = (struct bucket) {
        .pair.key = key,
        .pair.value = value,
        .tree.prev = buckets
    };

    insert_bucket(table, bucket);
    hashmap->buckets.count++;

//    if (hashmap->hashing.trees[index] == NULL) {
        hashmap->hashing.used++;
//    }

    hashmap->hashing.trees[index] = bucket;
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

    if (found == NULL) {
        return valueI_nil;
    } else {
        return found->pair.value;
    }
}

bool tableI_remove(morphine_instance_t I, struct table *table, struct value key) {
    if (table == NULL) {
        throwI_error(I, "Table is null");
    }

    struct hashmap *hashmap = &table->hashmap;

    if (hashmap->hashing.size == 0) {
        return false;
    }

    uint64_t hash = hashcode(I, key);
    size_t index = hash % hashmap->hashing.size;

    struct bucket *buckets = hashmap->hashing.trees[index];
    if (buckets != NULL) {
        struct bucket *last = NULL;
        struct bucket *current = buckets;
        while (current != NULL) {
            if (valueI_equal(I, current->pair.key, key)) {
                if (last == NULL) {
                    hashmap->hashing.trees[index] = current->tree.prev;
                } else {
                    last->tree.prev = current->tree.prev;
                }

                remove_bucket(table, current);
                allocI_free(I, current);
                hashmap->buckets.count--;

                return true;
            }

            last = current;
            current = current->tree.prev;
        }
    }

    return false;
}

void tableI_clear(morphine_instance_t I, struct table *table) {
    if (table == NULL) {
        throwI_error(I, "Table is null");
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

struct pair tableI_next(
    morphine_instance_t I,
    struct table *table,
    struct value *key,
    bool *next
) {
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
