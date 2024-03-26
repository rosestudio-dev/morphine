//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"

struct closure {
    struct object header;

    struct value callable;

    size_t size;
    struct value *values;
};

struct closure *closureI_create(morphine_instance_t, struct value callable, size_t size);
void closureI_free(morphine_instance_t, struct closure *);

struct value closureI_get(morphine_state_t, struct closure *, size_t index);
void closureI_set(morphine_state_t, struct closure *, size_t index, struct value value);

size_t closureI_allocated_size(struct closure *);
