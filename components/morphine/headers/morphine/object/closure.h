//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"

struct closure {
    struct object header;

    ml_size size;
    struct value callable;
    struct value *values;
};

struct closure *closureI_create(morphine_instance_t, struct value callable, ml_size);
void closureI_free(morphine_instance_t, struct closure *);

struct closure *closureI_packer_create(morphine_instance_t, ml_size);
void closureI_packer_init(morphine_instance_t, struct closure *, struct value callable);

struct value closureI_get(morphine_instance_t, struct closure *, ml_size);
void closureI_set(morphine_instance_t, struct closure *, ml_size, struct value);
