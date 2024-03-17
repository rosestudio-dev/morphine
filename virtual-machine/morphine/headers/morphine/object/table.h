//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"

struct table {
    struct object header;

    struct table *metatable;
    struct hashmap *hashmap;
};


struct table *tableI_create(morphine_instance_t, size_t size);
void tableI_free(morphine_instance_t, struct table *);

size_t tableI_size(morphine_instance_t, struct table *);
void tableI_set(morphine_instance_t, struct table *, struct value key, struct value value);
struct value tableI_get(morphine_instance_t, struct table *, struct value key, bool *has);
bool tableI_remove(morphine_instance_t, struct table *, struct value key);

size_t tableI_allocated_size(struct table *);
