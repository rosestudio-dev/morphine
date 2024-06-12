//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"

struct closure {
    struct object header;

    struct value callable;

    ml_size size;
    struct value *values;
};

struct closure *closureI_create(morphine_instance_t, struct value callable, ml_size size);
void closureI_free(morphine_instance_t, struct closure *);

struct value closureI_get(morphine_instance_t, struct closure *, ml_size index);
void closureI_set(morphine_instance_t, struct closure *, ml_size index, struct value value);
