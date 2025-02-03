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

void closureI_packer_vectorize(struct closure *closure, struct packer_vectorize *V) {
    packerI_vectorize_append(V, closure->callable);
    for (ml_size i = 0; i < closure->size; i++) {
        packerI_vectorize_append(V, closure->values[i]);
    }
}

void closureI_packer_write_info(struct closure *closure, struct packer_write *W) {
    packerI_write_ml_size(W, closure->size);
}

void closureI_packer_write_data(struct closure *closure, struct packer_write *W) {
    packerI_write_value(W, closure->callable);
    for (ml_size i = 0; i < closure->size; i++) {
        packerI_write_value(W, closure->values[i]);
    }
}

struct closure *closureI_packer_read_info(morphine_instance_t I, struct packer_read *R) {
    ml_size size = packerI_read_ml_size(R);
    return create_closure(I, valueI_nil, size);
}

void closureI_packer_read_data(morphine_instance_t I, struct closure *closure, struct packer_read *R) {
    struct value callable = callstackI_extract_callable(I, packerI_read_value(R));
    gcI_barrier(I, closure, callable);
    closure->callable = callable;

    for (ml_size i = 0; i < closure->size; i++) {
        gcI_safe_enter(I);
        struct value value = gcI_safe(I, packerI_read_value(R));
        closureI_set(I, closure, i, value);
        gcI_safe_exit(I);
    }
}
