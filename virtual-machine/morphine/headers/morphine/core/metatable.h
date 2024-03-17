//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "value.h"

#define MFS_START (MF_CALL)
#define MFS_COUNT (MF_GC + 1)

enum metatable_field {
    MF_CALL, MF_GET, MF_SET, MF_TO_STRING, MF_TYPE,

    MF_ADD, MF_SUB, MF_MUL, MF_DIV, MF_MOD,
    MF_EQUAL, MF_LESS, MF_LESS_EQUAL, MF_OR, MF_AND,
    MF_CONCAT,

    MF_NEGATE, MF_NOT, MF_LENGTH,
    MF_REF, MF_DEREF,

    // control
    MF_MASK, MF_GC
};

void metatableI_set(morphine_state_t, struct value, struct table *);
void metatableI_set_default(morphine_instance_t, enum value_type, struct table *);

struct value metatableI_get(morphine_state_t, struct value);
struct value metatableI_get_default(morphine_instance_t, enum value_type);

bool metatableI_test(morphine_instance_t, struct value, enum metatable_field, struct value *result);

const char *metatableI_field2string(morphine_instance_t, enum metatable_field);
enum metatable_field metatableI_string2field(morphine_state_t, const char *name);
