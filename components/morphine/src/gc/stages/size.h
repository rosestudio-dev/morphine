//
// Created by why-iskra on 16.04.2024.
//

#pragma once

#include "morphine/core/throw.h"
#include "morphine/object/closure.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/exception.h"
#include "morphine/object/function.h"
#include "morphine/object/native.h"
#include "morphine/object/reference.h"
#include "morphine/object/stream.h"
#include "morphine/object/string.h"
#include "morphine/object/table.h"
#include "morphine/object/userdata.h"
#include "morphine/object/vector.h"

static inline size_t size_table(struct table *table) {
    return sizeof(struct table) + ((size_t) table->hashmap.buckets.count) * sizeof(struct bucket)
           + table->hashmap.hashing.size * sizeof(struct tree);
}

static inline size_t size_closure(mattr_unused struct closure *closure) {
    return sizeof(struct closure) + ((size_t) closure->size) * sizeof(struct value);
}

static inline size_t size_vector(struct vector *vector) {
    return sizeof(struct vector) + ((size_t) vector->size.real) * sizeof(struct value);
}

static inline size_t size_function(struct function *function) {
    return sizeof(struct function) + ((size_t) function->instructions_count) * sizeof(morphine_instruction_t)
           + ((size_t) function->constants_count) * sizeof(struct value);
}

static inline size_t size_userdata(mattr_unused struct userdata *userdata) {
    return sizeof(struct userdata);
}

static inline size_t size_coroutine(struct coroutine *coroutine) {
    return sizeof(struct coroutine) + ((size_t) coroutine->stack.size) * sizeof(struct value)
           + ((size_t) coroutine->callstack.size) * sizeof(struct callframe);
}

static inline size_t size_native(mattr_unused struct native *native) {
    return sizeof(struct native);
}

static inline size_t size_exception(struct exception *exception) {
    return sizeof(struct exception) + ((size_t) exception->stacktrace.size) * sizeof(struct stacktrace_element);
}

static inline size_t size_string(struct string *string) {
    return sizeof(struct string) + (((size_t) string->size) + 1) * sizeof(char);
}

static inline size_t size_reference(mattr_unused struct reference *reference) {
    return sizeof(struct reference);
}

static inline size_t size_stream(mattr_unused struct stream *stream) {
    return sizeof(struct stream);
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
        case OBJ_TYPE_EXCEPTION: {
            return size_exception(cast(struct exception *, obj));
        }
        case OBJ_TYPE_STRING: {
            return size_string(cast(struct string *, obj));
        }
        case OBJ_TYPE_REFERENCE: {
            return size_reference(cast(struct reference *, obj));
        }
        case OBJ_TYPE_STREAM: {
            return size_stream(cast(struct stream *, obj));
        }
    }

    throwI_panic(I, "unsupported object");
}
