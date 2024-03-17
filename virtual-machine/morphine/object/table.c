//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/table.h"
#include "morphine/object/state.h"
#include "morphine/core/alloc.h"
#include "morphine/core/throw.h"
#include "morphine/core/hashmap.h"
#include "morphine/core/gc.h"

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
