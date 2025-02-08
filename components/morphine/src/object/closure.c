//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/closure.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"
#include "morphine/object/coroutine.h"

static inline struct closure *create_closure(morphine_instance_t I, struct value callable, ml_size size) {
    gcI_safe_enter(I);
    gcI_safe(I, callable);

    // create
    struct closure *result = allocI_uni(I, NULL, sizeof(struct closure));
    (*result) = (struct closure) {
        .size = 0,
        .callable = callable,
        .values = NULL,
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

struct closure *closureI_create(morphine_instance_t I, struct value callable, ml_size size) {
    return create_closure(I, callstackI_extract_callable(I, callable), size);
}

void closureI_free(morphine_instance_t I, struct closure *closure) {
    allocI_free(I, closure->values);
    allocI_free(I, closure);
}

struct closure *closureI_packer_create(morphine_instance_t I, ml_size size) {
    return create_closure(I, valueI_nil, size);
}

void closureI_packer_init(morphine_instance_t I, struct closure *closure, struct value callable) {
    closure->callable = callstackI_extract_callable(I, callable);
}

struct value closureI_get(morphine_instance_t I, struct closure *closure, ml_size index) {
    if (index >= closure->size) {
        throwI_error(I, "closure index was out of bounce");
    }

    return closure->values[index];
}

void closureI_set(morphine_instance_t I, struct closure *closure, ml_size index, struct value value) {
    if (index >= closure->size) {
        throwI_error(I, "closure index was out of bounce");
    }

    closure->values[index] = gcI_valbarrier(I, closure, value);
}
