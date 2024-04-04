//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include <stddef.h>
#include "morphine/core/value.h"
#include "morphine/core/instruction.h"

struct uuid {
    uint64_t most_significant_bits;
    uint64_t least_significant_bits;
};

struct function {
    struct object header;

    struct uuid uuid;
    char *name;

    size_t name_len;

    size_t instructions_count;
    size_t constants_count;
    size_t arguments_count;
    size_t slots_count;
    size_t closures_count;
    size_t statics_count;
    size_t params_count;

    instruction_t *instructions;
    struct value *constants;

    struct value *statics;

    struct value registry_key;
};

bool functionI_uuid_equal(struct uuid a, struct uuid b);

struct function *functionI_create(
    morphine_instance_t,
    struct uuid uuid,
    size_t name_len,
    size_t constants_count,
    size_t instructions_count,
    size_t statics_count
);

void functionI_free(morphine_instance_t, struct function *);

struct value functionI_static_get(morphine_instance_t, struct function *, size_t index);
void functionI_static_set(morphine_instance_t, struct function *, size_t index, struct value value);

struct value functionI_constant_get(morphine_instance_t, struct function *, size_t index);
void functionI_constant_set(morphine_instance_t, struct function *, size_t index, struct value value);
