//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"

#define MFS_START (MF_TYPE)
#define MFS_COUNT (MF_GC + 1)

enum metatable_field {
    // operations
    MF_TYPE,          // (callable | value)
    MF_CALL,          // callable
    MF_GET,           // (callable | value)
    MF_SET,           // callable
    MF_TO_STRING,     // (callable | value)
    MF_ADD,           // (callable | value)
    MF_SUB,           // (callable | value)
    MF_MUL,           // (callable | value)
    MF_DIV,           // (callable | value)
    MF_MOD,           // (callable | value)
    MF_EQUAL,         // (callable | value)
    MF_LESS,          // (callable | value)
    MF_OR,            // (callable | value)
    MF_AND,           // (callable | value)
    MF_CONCAT,        // (callable | value)
    MF_NEGATE,        // (callable | value)
    MF_NOT,           // (callable | value)
    MF_LENGTH,        // (callable | value)
    MF_REF,           // (callable | value)
    MF_DEREF,         // (callable | value)
    MF_ITERATOR,      // (callable | value)
    MF_ITERATOR_INIT, // callable
    MF_ITERATOR_HAS,  // (callable | value)
    MF_ITERATOR_NEXT, // (callable | value)

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
