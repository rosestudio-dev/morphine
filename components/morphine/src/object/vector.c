//
// Created by why-iskra on 22.04.2024.
//

#include "morphine/object/vector.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"
#include "morphine/object/userdata.h"
#include "morphine/utils/overflow.h"
#include <memory.h>

#define min(a, b) ((a) > (b) ? (b) : (a))

struct vector *vectorI_create(morphine_instance_t I, ml_size size) {
    struct vector *result = allocI_uni(I, NULL, sizeof(struct vector));
    (*result) = (struct vector) {
        .mode.fixed = true,
        .mode.mutable = true,
        .mode.accessible = true,
        .lock.mode = false,
        .size.real = 0,
        .size.accessible = 0,
        .values = NULL,
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_VECTOR);

    // config
    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(result));

    result->values = allocI_vec(I, NULL, size, sizeof(struct value));
    result->size.accessible = size;
    result->size.real = size;

    for (ml_size i = 0; i < size; i++) {
        result->values[i] = valueI_nil;
    }

    gcI_safe_exit(I);

    return result;
}

void vectorI_free(morphine_instance_t I, struct vector *vector) {
    allocI_free(I, vector->values);
    allocI_free(I, vector);
}

void vectorI_mode_fixed(morphine_instance_t I, struct vector *vector, bool is_fixed) {
    if (vector->lock.mode) {
        throwI_error(I, "vector is locked");
    }

    if (!vector->mode.fixed && is_fixed) {
        vector->values = allocI_vec(I, vector->values, vector->size.accessible, sizeof(struct value));
        vector->size.real = vector->size.accessible;
    }

    vector->mode.fixed = is_fixed;
}

void vectorI_mode_mutable(morphine_instance_t I, struct vector *vector, bool is_mutable) {
    if (vector->lock.mode) {
        throwI_error(I, "vector is locked");
    }

    vector->mode.mutable = is_mutable;
}

void vectorI_mode_accessible(morphine_instance_t I, struct vector *vector, bool is_accessible) {
    if (vector->lock.mode) {
        throwI_error(I, "vector is locked");
    }

    vector->mode.accessible = is_accessible;
}

void vectorI_lock_mode(struct vector *vector) {
    vector->lock.mode = true;
}

ml_size vectorI_size(struct vector *vector) {
    return vector->size.accessible;
}

void vectorI_set(morphine_instance_t I, struct vector *vector, ml_size index, struct value value) {
    if (!vector->mode.mutable) {
        throwI_error(I, "vector is immutable");
    }

    if (index >= vector->size.accessible) {
        throwI_error(I, "vector index out of bounce");
    }

    vector->values[index] = gcI_valbarrier(I, vector, value);
}

void vectorI_add(morphine_instance_t I, struct vector *vector, ml_size index, struct value value) {
    if (!vector->mode.mutable) {
        throwI_error(I, "vector is immutable");
    }

    if (vector->mode.fixed) {
        throwI_error(I, "vector is fixed");
    }

    if (index > vector->size.accessible) {
        throwI_error(I, "vector index out of bounce");
    }

    if (vector->size.accessible == vector->size.real) {
        gcI_safe_enter(I);
        gcI_safe(I, valueI_object(vector));
        gcI_safe(I, value);

        ml_size new_size = mm_overflow_opc_add(
            vector->size.real,
            MPARAM_VECTOR_AMORTIZATION,
            throwI_error(I, "vector size limit has been exceeded")
        );

        vector->values = allocI_vec(I, vector->values, new_size, sizeof(struct value));

        vector->size.real += MPARAM_VECTOR_AMORTIZATION;
        gcI_safe_exit(I);
    }

    vector->size.accessible++;
    for (ml_size i = 0; i < vector->size.accessible - 1 - index; i++) {
        ml_size from = vector->size.accessible - i - 2;
        ml_size to = vector->size.accessible - i - 1;
        vector->values[to] = vector->values[from];
    }

    vector->values[index] = gcI_valbarrier(I, vector, value);
}

bool vectorI_has(struct vector *vector, struct value value) {
    for (ml_size i = 0; i < vector->size.accessible; i++) {
        if (valueI_compare(vector->values[i], value) == 0) {
            return true;
        }
    }

    return false;
}

struct value vectorI_get(morphine_instance_t I, struct vector *vector, ml_size index) {
    if (!vector->mode.accessible) {
        throwI_error(I, "vector is inaccessible");
    }

    if (index >= vector->size.accessible) {
        throwI_error(I, "vector index out of bounce");
    }

    return vector->values[index];
}

struct value vectorI_remove(morphine_instance_t I, struct vector *vector, ml_size index) {
    if (!vector->mode.mutable) {
        throwI_error(I, "vector is immutable");
    }

    if (vector->mode.fixed) {
        throwI_error(I, "vector is fixed");
    }

    if (index >= vector->size.accessible) {
        throwI_error(I, "vector index out of bounce");
    }

    struct value value = vector->values[index];

    for (ml_size i = index; i < vector->size.accessible - 1; i++) {
        vector->values[i] = vector->values[i + 1];
    }

    vector->size.accessible--;

    if (vector->size.real - vector->size.accessible > MPARAM_VECTOR_AMORTIZATION) {
        gcI_safe_enter(I);
        gcI_safe(I, valueI_object(vector));
        gcI_safe(I, value);

        vector->values =
            allocI_vec(I, vector->values, vector->size.real - MPARAM_VECTOR_AMORTIZATION, sizeof(struct value));

        vector->size.real -= MPARAM_VECTOR_AMORTIZATION;
        gcI_safe_exit(I);
    }

    return value;
}

void vectorI_resize(morphine_instance_t I, struct vector *vector, ml_size size) {
    if (!vector->mode.mutable) {
        throwI_error(I, "vector is immutable");
    }

    if (vector->mode.fixed) {
        throwI_error(I, "vector is fixed");
    }

    vector->values = allocI_vec(I, vector->values, size, sizeof(struct value));

    if (size > vector->size.accessible) {
        for (ml_size i = vector->size.accessible; i < size; i++) {
            vector->values[i] = valueI_nil;
        }
    }

    vector->size.accessible = size;
    vector->size.real = size;
}

struct vector *vectorI_copy(morphine_instance_t I, struct vector *vector) {
    if (!vector->mode.accessible) {
        throwI_error(I, "vector is inaccessible");
    }

    ml_size size = vector->size.accessible;

    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(vector));
    struct vector *result = gcI_safe_obj(I, vector, vectorI_create(I, size));
    memcpy(result->values, vector->values, ((size_t) size) * sizeof(struct value));

    for (ml_size i = 0; i < size; i++) {
        gcI_valbarrier(I, result, result->values[i]);
    }

    result->mode.accessible = vector->mode.accessible;
    result->mode.mutable = vector->mode.mutable;
    result->mode.fixed = vector->mode.fixed;

    gcI_safe_exit(I);

    return result;
}

struct vector *vectorI_concat(morphine_instance_t I, struct vector *a, struct vector *b) {
    ml_size size =
        mm_overflow_opc_add(a->size.accessible, b->size.accessible, throwI_error(I, "too big concat vector length"));

    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(a));
    gcI_safe(I, valueI_object(b));
    struct vector *result = gcI_safe_obj(I, vector, vectorI_create(I, size));

    memcpy(result->values, a->values, ((size_t) a->size.accessible) * sizeof(struct value));
    memcpy(result->values + a->size.accessible, b->values, ((size_t) b->size.accessible) * sizeof(struct value));

    for (ml_size i = 0; i < size; i++) {
        gcI_valbarrier(I, result, result->values[i]);
    }

    result->mode.accessible = a->mode.accessible;
    result->mode.mutable = a->mode.mutable;
    result->mode.fixed = a->mode.fixed;

    gcI_safe_exit(I);

    return result;
}

void vectorI_packer_vectorize(morphine_instance_t I, struct vector *vector, struct packer_vectorize *V) {
    if (!vector->mode.accessible) {
        throwI_error(I, "vector is inaccessible");
    }

    for (ml_size i = 0; i < vector->size.accessible; i++) {
        packerI_vectorize_append(V, vector->values[i]);
    }
}

void vectorI_packer_write_info(morphine_instance_t I, struct vector *vector, struct packer_write *W) {
    if (!vector->mode.accessible) {
        throwI_error(I, "vector is inaccessible");
    }

    packerI_write_ml_size(W, vector->size.accessible);
}

void vectorI_packer_write_data(morphine_instance_t I, struct vector *vector, struct packer_write *W) {
    if (!vector->mode.accessible) {
        throwI_error(I, "vector is inaccessible");
    }

    for (ml_size i = 0; i < vector->size.accessible; i++) {
        packerI_write_value(W, vector->values[i]);
    }

    packerI_write_bool(W, vector->mode.mutable);
    packerI_write_bool(W, vector->mode.fixed);
    packerI_write_bool(W, vector->mode.accessible);
    packerI_write_bool(W, vector->lock.mode);
}

struct vector *vectorI_packer_read_info(morphine_instance_t I, struct packer_read *R) {
    ml_size size = packerI_read_ml_size(R);
    return vectorI_create(I, size);
}

void vectorI_packer_read_data(morphine_instance_t I, struct vector *vector, struct packer_read *R) {
    for (ml_size i = 0; i < vector->size.accessible; i++) {
        gcI_safe_enter(I);
        struct value value = gcI_safe(I, packerI_read_value(R));
        vectorI_set(I, vector, i, value);
        gcI_safe_exit(I);
    }

    vectorI_mode_mutable(I, vector, packerI_read_bool(R));
    vectorI_mode_fixed(I, vector, packerI_read_bool(R));
    vectorI_mode_accessible(I, vector, packerI_read_bool(R));

    if (packerI_read_bool(R)) {
        vectorI_lock_mode(vector);
    }
}
