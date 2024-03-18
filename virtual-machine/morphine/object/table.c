//
// Created by whyiskra on 16.12.23.
//

#include <memory.h>
#include "morphine/object/table.h"
#include "morphine/object/string.h"
#include "morphine/object/state.h"
#include "morphine/core/alloc.h"
#include "morphine/core/throw.h"
#include "morphine/core/gc.h"
#include "morphine/config/hashmap.h"

struct bucket {
    uint64_t hash;
    uint16_t dib;

    struct pair pair;
};

struct hashmap {
    size_t cap;
    size_t size;
    size_t count;
    size_t growat;
    size_t shrinkat;
    struct bucket *buckets;
};

// region hashmap

static inline uint64_t hashcode(struct value value) {
    struct string *str = valueI_safe_as_string(value, NULL);
    if (str != NULL) {
        uint64_t h = 0;
        for (size_t i = 0; i < str->size; i++) {
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
        case VALUE_TYPE_STATE:
        case VALUE_TYPE_PROTO:
        case VALUE_TYPE_NATIVE:
        case VALUE_TYPE_REFERENCE:
            return (uint64_t) valueI_as_object(value);
    }
}

static inline struct hashmap *hashmap_new(morphine_instance_t I, size_t cap) {
    const size_t ncap = 16;
    if (cap < ncap) {
        cap = ncap;
    }

    struct hashmap *map = allocI_uni(I, NULL, 0, sizeof(struct hashmap));

    map->count = 0;

    map->cap = cap;
    map->size = cap;

    map->growat = map->size * HASHMAP_GROW_FACTOR_PERCENT / 100;
    map->shrinkat = map->size * HASHMAP_SHRINK_FACTOR_PERCENT / 100;

    map->buckets = allocI_uni(I, NULL, 0, sizeof(struct bucket) * map->size);
    memset(map->buckets, 0, sizeof(struct bucket) * map->size);

    return map;
}

static inline void hashmap_clear(morphine_instance_t I, struct hashmap *map, bool update_cap) {
    map->count = 0;

    if (update_cap) {
        map->cap = map->size;
    } else if (map->size != map->cap) {
        void *new_buckets = allocI_uni(I, NULL, 0, sizeof(struct bucket) * map->cap);

        allocI_uni(I, map->buckets, sizeof(struct bucket) * map->size, 0);

        map->buckets = new_buckets;
        map->size = map->cap;
    }

    memset(map->buckets, 0, sizeof(struct bucket) * map->size);

    map->growat = map->size * HASHMAP_GROW_FACTOR_PERCENT / 100;
    map->shrinkat = map->size * HASHMAP_SHRINK_FACTOR_PERCENT / 100;
}

static inline void resize(morphine_instance_t I, struct hashmap *map, size_t new_cap) {
    struct hashmap *newmap = hashmap_new(I, new_cap);

    for (size_t i = 0; i < map->size; i++) {
        struct bucket *entry = map->buckets + i;

        if (!entry->dib) {
            continue;
        }

        entry->dib = 1;
        size_t j = entry->hash % newmap->size;

        for(;;) {
            struct bucket *bucket = newmap->buckets + j;

            if (bucket->dib == 0) {
                *bucket = *entry;
                break;
            }

            if (bucket->dib < entry->dib) {
                struct bucket temp = *bucket;
                *bucket = *entry;
                *entry = temp;
            }

            j = (j + 1) % newmap->size;

            entry->dib ++;
        }
    }

    allocI_uni(I, map->buckets, sizeof(struct bucket) * map->size, 0);

    map->buckets = newmap->buckets;
    map->size = newmap->size;
    map->growat = newmap->growat;
    map->shrinkat = newmap->shrinkat;

    allocI_uni(I, newmap, sizeof(struct hashmap), 0);
}

static inline bool hashmap_set(morphine_instance_t I, struct hashmap *map, struct pair item) {
    if (morphinem_unlikely(map->count >= map->growat)) {
        resize(I, map, map->size * (1 << HASHMAP_GROWPOWER));
    }

    struct bucket entry = {
        .hash = hashcode(item.key),
        .dib = 1,
        .pair = item
    };

    size_t i = entry.hash % map->size;
    for(;;) {
        struct bucket *bucket = map->buckets + i;

        if (morphinem_unlikely(bucket->dib == 0)) {
            *bucket = entry;
            map->count++;
            return false;
        }

        if (morphinem_likely(entry.hash == bucket->hash && valueI_equal(I, entry.pair.key, bucket->pair.key))) {
            bucket->pair = entry.pair;
            return true;
        }

        if (bucket->dib < entry.dib) {
            struct bucket temp = *bucket;
            *bucket = entry;
            entry = temp;
        }

        i = (i + 1) % map->size;

        entry.dib++;
    }
}

static inline struct value hashmap_get(morphine_instance_t I, struct hashmap *map, struct value key, bool *has) {
    uint64_t hash = hashcode(key);

    size_t i = hash % map->size;
    for(;;) {
        struct bucket *bucket = map->buckets + i;

        if (morphinem_unlikely(!bucket->dib)) {
            if (has != NULL) {
                *has = false;
            }

            return valueI_nil;
        }

        if (morphinem_likely(bucket->hash == hash && valueI_equal(I, key, bucket->pair.key))) {
            if (has != NULL) {
                *has = true;
            }

            return bucket->pair.value;
        }

        i = (i + 1) % map->size;
    }
}

static inline bool hashmap_delete(morphine_instance_t I, struct hashmap *map, struct value key) {
    uint64_t hash = hashcode(key);

    size_t i = hash % map->size;
    for(;;) {
        struct bucket *bucket = map->buckets + i;

        if (!bucket->dib) {
            return false;
        }

        if (bucket->hash == hash && valueI_equal(I, key, bucket->pair.key)) {
            bucket->dib = 0;

            for(;;) {
                struct bucket *prev = bucket;

                i = (i + 1) % map->size;
                bucket = map->buckets + i;

                if (bucket->dib <= 1) {
                    prev->dib = 0;
                    break;
                }

                *prev = *bucket;

                prev->dib--;
            }

            map->count--;

            if (map->size > map->cap && map->count <= map->shrinkat) {
                resize(I, map, map->size / 2);
            }

            return true;
        }

        i = (i + 1) % map->size;
    }
}

static inline size_t hashmap_count(struct hashmap *map) {
    return map->count;
}

static inline size_t hashmap_allocated_size(struct hashmap *map) {
    return map->size * sizeof(struct bucket) + sizeof(struct hashmap);
}

static inline void hashmap_free(morphine_instance_t I, struct hashmap *map) {
    allocI_uni(I, map->buckets, sizeof(struct bucket) * map->size, 0);
    allocI_uni(I, map, sizeof(struct hashmap), 0);
}

static inline bool hashmap_iter(struct hashmap *map, size_t *i, struct pair *item) {
    struct bucket *bucket;

    do {
        if (*i >= map->size) {
            return false;
        }

        bucket = map->buckets + (*i);
        (*i) ++;
    } while (!bucket->dib);

    *item = bucket->pair;

    return true;
}

// endregion

struct table *tableI_create(morphine_instance_t I, size_t size) {
    size_t alloc_size = sizeof(struct table);

    struct table *result = allocI_uni(I, NULL, 0, alloc_size);

    (*result) = (struct table) {
        .metatable = NULL,
        .hashmap = hashmap_new(I, size)
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_TABLE);

    return result;
}

void tableI_free(morphine_instance_t I, struct table *table) {
    hashmap_free(I, table->hashmap);

    allocI_uni(
        I,
        table,
        sizeof(struct table),
        0
    );
}

size_t tableI_allocated_size(struct table *table) {
    return sizeof(struct table) + hashmap_allocated_size(table->hashmap);
}

size_t tableI_size(morphine_instance_t I, struct table *table) {
    if (table == NULL) {
        throwI_message_panic(I, NULL, "Table is null");
    }

    return hashmap_count(table->hashmap);
}

void tableI_set(morphine_instance_t I, struct table *table, struct value key, struct value value) {
    if (table == NULL) {
        throwI_message_panic(I, NULL, "Table is null");
    }

    gcI_barrier(table, key);
    gcI_barrier(table, value);

    hashmap_set(I, table->hashmap, (struct pair) { .key = key, .value = value });
}

struct value tableI_get(morphine_instance_t I, struct table *table, struct value key, bool *has) {
    if (table == NULL) {
        throwI_message_panic(I, NULL, "Table is null");
    }

    return hashmap_get(I, table->hashmap, key, has);
}

bool tableI_remove(morphine_instance_t I, struct table *table, struct value key) {
    if (table == NULL) {
        throwI_message_panic(I, NULL, "Table is null");
    }

    return hashmap_delete(I, table->hashmap, key);
}

bool tableI_iter(morphine_instance_t I, struct table *table, size_t *i, struct pair *item) {
    if (table == NULL) {
        throwI_message_panic(I, NULL, "Table is null");
    }

    return hashmap_iter(table->hashmap, i, item);
}
