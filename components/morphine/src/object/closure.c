//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/closure.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"

struct closure *closureI_create(morphine_instance_t I, struct value callable, ml_size size) {
    struct value extracted = callstackI_extract_callable(I, callable);

    gcI_safe_enter(I);
    gcI_safe(I, extracted);

    // create
    struct closure *result = allocI_uni(I, NULL, sizeof(struct closure));
    (*result) = (struct closure) {
        .size = 0,
        .callable = extracted,
        .values = NULL
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_CLOSURE);

    // config
    gcI_safe(I, valueI_object(result));

    result->values = allocI_vec(I, NULL, size, sizeof(struct value));
    result->size = size;

    for (ml_size i = 0; i < size; i++) {
        result->values[i] = valueI_nil;
    }

    gcI_safe_exit(I);

    return result;
}

void closureI_free(morphine_instance_t I, struct closure *closure) {
    allocI_free(I, closure->values);
    allocI_free(I, closure);
}

struct value closureI_get(morphine_instance_t I, struct closure *closure, ml_size index) {
    if (closure == NULL) {
        throwI_error(I, "closure is null");
    }

    if (index >= closure->size) {
        throwI_error(I, "closure index was out of bounce");
    }

    return closure->values[index];
}

void closureI_set(morphine_instance_t I, struct closure *closure, ml_size index, struct value value) {
    if (closure == NULL) {
        throwI_error(I, "closure is null");
    }

    if (index >= closure->size) {
        throwI_error(I, "closure index was out of bounce");
    }

    gcI_barrier(I, closure, value);
    closure->values[index] = value;
}
