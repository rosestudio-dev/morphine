//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"

#define MFS_START (MF_TYPE)
#define MFS_COUNT (MF_GC + 1)

enum metatable_field {
    // operations
    MF_TYPE,          // callable
    MF_CALL,          // callable
    MF_GET,           // callable
    MF_SET,           // callable
    MF_TO_STRING,     // callable
    MF_ADD,           // callable
    MF_SUB,           // callable
    MF_MUL,           // callable
    MF_DIV,           // callable
    MF_MOD,           // callable
    MF_EQUAL,         // callable
    MF_LESS,          // callable
    MF_OR,            // callable
    MF_AND,           // callable
    MF_CONCAT,        // callable
    MF_NEGATE,        // callable
    MF_NOT,           // callable
    MF_LENGTH,        // callable
    MF_REF,           // callable
    MF_DEREF,         // callable
    MF_ITERATOR,      // callable
    MF_ITERATOR_INIT, // callable
    MF_ITERATOR_HAS,  // callable
    MF_ITERATOR_NEXT, // callable

    // control
    MF_MASK, // value
    MF_GC    // callable
};

void metatableI_set(morphine_instance_t, struct value, struct table *);
void metatableI_set_default(morphine_instance_t, enum value_type, struct table *);

struct value metatableI_get(morphine_instance_t, struct value);
struct value metatableI_get_default(morphine_instance_t, enum value_type);

bool metatableI_test(morphine_instance_t, struct value, enum metatable_field, struct value *result);

const char *metatableI_field2string(morphine_instance_t, enum metatable_field);
enum metatable_field metatableI_string2field(morphine_instance_t, const char *name);
