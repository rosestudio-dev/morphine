//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/closure.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"
#include "morphine/object/coroutine.h"

static inline struct closure *create_closure(morphine_instance_t I, struct value callable, struct value value) {
    gcI_safe_enter(I);
    gcI_safe(I, callable);
    gcI_safe(I, value);

    // create
    struct closure *result = allocI_uni(I, NULL, sizeof(struct closure));
    (*result) = (struct closure) {
        .lock = false,
        .callable = callable,
        .value = value,
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_CLOSURE);

    gcI_safe_exit(I);

    return result;
}

struct closure *closureI_create(morphine_instance_t I, struct value callable, struct value value) {
    return create_closure(I, callstackI_extract_callable(I, callable), value);
}

void closureI_free(morphine_instance_t I, struct closure *closure) {
    allocI_free(I, closure);
}

void closureI_lock(morphine_instance_t I, struct closure *closure) {
    if (closure == NULL) {
        throwI_error(I, "closure is null");
    }

    closure->lock = true;
}

static bool check_lock(morphine_instance_t I, struct closure *closure) {
    if (!closure->lock) {
        return false;
    }

    struct callframe *frame = callstackI_interpreter_context(I);
    return frame == NULL || valueI_safe_as_closure(*frame->s.direct.callable, NULL) != closure;
}

void closureI_unlock(morphine_instance_t I, struct closure *closure) {
    if (closure == NULL) {
        throwI_error(I, "closure is null");
    }

    if (check_lock(I, closure)) {
        throwI_error(I, "unable to unlock closure");
    }

    closure->lock = false;
}

struct value closureI_value(morphine_instance_t I, struct closure *closure) {
    if (closure == NULL) {
        throwI_error(I, "closure is null");
    }

    if (check_lock(I, closure)) {
        throwI_error(I, "unable to access closure");
    }

    return closure->value;
}

void closureI_packer_vectorize(morphine_instance_t I, struct closure *closure, struct packer_vectorize *V) {
    if (check_lock(I, closure)) {
        throwI_error(I, "unable to pack closure");
    }

    packerI_vectorize_append(V, closure->callable);
    packerI_vectorize_append(V, closure->value);
}

void closureI_packer_write_info(
    morphine_instance_t I,
    morphine_unused struct closure *closure,
    morphine_unused struct packer_write *W
) {
    if (check_lock(I, closure)) {
        throwI_error(I, "unable to pack closure");
    }
}

void closureI_packer_write_data(morphine_instance_t I, struct closure *closure, struct packer_write *W) {
    if (check_lock(I, closure)) {
        throwI_error(I, "unable to pack closure");
    }

    packerI_write_value(W, closure->callable);
    packerI_write_value(W, closure->value);
}

struct closure *closureI_packer_read_info(morphine_instance_t I, morphine_unused struct packer_read *R) {
    return create_closure(I, valueI_nil, valueI_nil);
}

void closureI_packer_read_data(morphine_instance_t I, struct closure *closure, struct packer_read *R) {
    struct value callable = callstackI_extract_callable(I, packerI_read_value(R));
    gcI_barrier(I, closure, callable);
    closure->callable = callable;

    struct value value = packerI_read_value(R);
    gcI_barrier(I, closure, value);
    closure->value = value;
}
