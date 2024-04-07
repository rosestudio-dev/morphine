//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/closure.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"

struct closure *closureI_create(morphine_instance_t I, struct value callable, size_t size) {
    struct closure *result = allocI_uni(I, NULL, sizeof(struct closure));

    (*result) = (struct closure) {
        .size = size,
        .callable = callable,
        .values = NULL
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_CLOSURE);

    gcI_safe_obj(I, objectI_cast(result));

    result->values = allocI_vec(I, NULL, size, sizeof(struct closure));
    result->size = size;

    for (size_t i = 0; i < size; i++) {
        result->values[i] = valueI_nil;
    }

    gcI_reset_safe(I);

    return result;
}

void closureI_free(morphine_instance_t I, struct closure *closure) {
    allocI_free(I, closure->values);
    allocI_free(I, closure);
}

struct value closureI_get(morphine_instance_t I, struct closure *closure, size_t index) {
    if (closure == NULL) {
        throwI_error(I, "Closure is null");
    }

    if (index >= closure->size) {
        throwI_error(I, "Closure index was out of bounce");
    }

    return closure->values[index];
}

void closureI_set(morphine_instance_t I, struct closure *closure, size_t index, struct value value) {
    if (closure == NULL) {
        throwI_error(I, "Closure is null");
    }

    if (index >= closure->size) {
        throwI_error(I, "Closure index was out of bounce");
    }

    gcI_barrier(closure, value);
    closure->values[index] = value;
}
