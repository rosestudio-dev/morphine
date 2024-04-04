//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/closure.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"

struct closure *closureI_create(morphine_instance_t I, struct value callable, size_t size) {
    size_t alloc_size = sizeof(struct closure) + size * sizeof(struct value);

    struct closure *result = allocI_uni(I, NULL, alloc_size);

    (*result) = (struct closure) {
        .size = size,
        .callable = callable,
        .values = ((void *) result) + sizeof(struct closure)
    };

    for (size_t i = 0; i < size; i++) {
        result->values[i] = valueI_nil;
    }

    objectI_init(I, objectI_cast(result), OBJ_TYPE_CLOSURE);

    return result;
}

void closureI_free(morphine_instance_t I, struct closure *closure) {
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
