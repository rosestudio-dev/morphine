//
// Created by why-iskra on 16.04.2024.
//

#pragma once

#include "morphine/object/coroutine.h"
#include "morphine/object/table.h"
#include "morphine/object/closure.h"
#include "morphine/object/userdata.h"
#include "morphine/object/function.h"
#include "morphine/object/reference.h"
#include "morphine/object/string.h"
#include "morphine/object/native.h"
#include "morphine/object/iterator.h"
#include "morphine/object/vector.h"
#include "morphine/object/sio.h"
#include "morphine/core/throw.h"
#include "morphine/utils/unused.h"

static inline size_t size_table(struct table *table) {
    return sizeof(struct table) +
           ((size_t) table->hashmap.buckets.count) * sizeof(struct bucket) +
           table->hashmap.hashing.size * sizeof(struct tree);
}

static inline size_t size_closure(struct closure *closure) {
    return sizeof(struct closure) +
           closure->size * sizeof(struct value);
}

static inline size_t size_vector(struct vector *vector) {
    return sizeof(struct vector) +
           ((size_t) vector->size.real) * sizeof(struct value);
}

static inline size_t size_function(struct function *function) {
    return sizeof(struct function) +
           (function->name_len + 1) * sizeof(char) +
           function->instructions_count * sizeof(instruction_t) +
           function->statics_count * sizeof(struct value) +
           function->constants_count * sizeof(struct value);
}

static inline size_t size_userdata(struct userdata *userdata) {
    return sizeof(struct userdata) +
           (userdata->name_len + 1) * sizeof(char) +
           userdata->size;
}

static inline size_t size_coroutine(struct coroutine *coroutine) {
    return sizeof(struct coroutine) +
           coroutine->stack.size * sizeof(struct value) +
           coroutine->callstack.size * sizeof(struct callstack);
}

static inline size_t size_native(struct native *native) {
    return sizeof(struct native) +
           (native->name_len + 1) * sizeof(char);
}

static inline size_t size_iterator(struct iterator *iterator) {
    unused(iterator);
    return sizeof(struct iterator);
}

static inline size_t size_string(struct string *string) {
    return sizeof(struct string) + (((size_t) string->size) + 1) * sizeof(char);
}

static inline size_t size_reference(struct reference *reference) {
    unused(reference);
    return sizeof(struct reference);
}

static inline size_t size_sio(struct sio *sio) {
    unused(sio);
    return sizeof(struct sio);
}

static inline size_t size_obj(morphine_instance_t I, struct object *obj) {
    switch (obj->type) {
        case OBJ_TYPE_TABLE: {
            return size_table(cast(struct table *, obj));
        }
        case OBJ_TYPE_VECTOR: {
            return size_vector(cast(struct vector *, obj));
        }
        case OBJ_TYPE_CLOSURE: {
            return size_closure(cast(struct closure *, obj));
        }
        case OBJ_TYPE_FUNCTION: {
            return size_function(cast(struct function *, obj));
        }
        case OBJ_TYPE_USERDATA: {
            return size_userdata(cast(struct userdata *, obj));
        }
        case OBJ_TYPE_COROUTINE: {
            return size_coroutine(cast(struct coroutine *, obj));
        }
        case OBJ_TYPE_NATIVE: {
            return size_native(cast(struct native *, obj));
        }
        case OBJ_TYPE_ITERATOR: {
            return size_iterator(cast(struct iterator *, obj));
        }
        case OBJ_TYPE_STRING: {
            return size_string(cast(struct string *, obj));
        }
        case OBJ_TYPE_REFERENCE: {
            return size_reference(cast(struct reference *, obj));
        }
        case OBJ_TYPE_SIO: {
            return size_sio(cast(struct sio *, obj));
        }
    }

    throwI_panic(I, "Unsupported object");
}
