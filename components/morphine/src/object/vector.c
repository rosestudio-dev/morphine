//
// Created by why-iskra on 22.04.2024.
//

#include "morphine/object/vector.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"
#include <memory.h>

static void resize(morphine_instance_t I, struct vector *vector, ml_size size) {
    vector->values = allocI_vec(I, vector->values, size, sizeof(struct value));

    if (size > vector->size.accessible) {
        for (ml_size i = vector->size.accessible; i < size; i++) {
            vector->values[i] = valueI_nil;
        }
    }

    vector->size.accessible = size;
    vector->size.real = size;
}

struct vector *vectorI_create(morphine_instance_t I, ml_size size, bool dynamic) {
    struct vector *result = allocI_uni(I, NULL, sizeof(struct vector));
    (*result) = (struct vector) {
        .dynamic = dynamic,
        .size.real = 0,
        .size.accessible = 0,
        .values = NULL,
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_VECTOR);

    // config
    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(result));

    resize(I, result, size);

    gcI_safe_exit(I);

    return result;
}

void vectorI_free(morphine_instance_t I, struct vector *vector) {
    allocI_free(I, vector->values);
    allocI_free(I, vector);
}

ml_size vectorI_size(struct vector *vector) {
    return vector->size.accessible;
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
    if (index >= vector->size.accessible) {
        throwI_error(I, "vector index out of bounce");
    }

    return vector->values[index];
}

void vectorI_set(morphine_instance_t I, struct vector *vector, ml_size index, struct value value) {
    if (index >= vector->size.accessible) {
        throwI_error(I, "vector index out of bounce");
    }

    vector->values[index] = gcI_valbarrier(I, vector, value);
}

void vectorI_add(morphine_instance_t I, struct vector *vector, ml_size index, struct value value) {
    if (!vector->dynamic) {
        throwI_error(I, "vector isn't dynamic");
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

struct value vectorI_remove(morphine_instance_t I, struct vector *vector, ml_size index) {
    if (!vector->dynamic) {
        throwI_error(I, "vector isn't dynamic");
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
    if (!vector->dynamic) {
        throwI_error(I, "vector isn't dynamic");
    }

    resize(I, vector, size);
}

struct vector *vectorI_copy(morphine_instance_t I, struct vector *vector) {
    ml_size size = vector->size.accessible;

    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(vector));
    struct vector *result = gcI_safe_obj(I, vector, vectorI_create(I, size, vector->dynamic));
    memcpy(result->values, vector->values, ((size_t) size) * sizeof(struct value));

    for (ml_size i = 0; i < size; i++) {
        gcI_valbarrier(I, result, result->values[i]);
    }

    gcI_safe_exit(I);

    return result;
}

struct vector *vectorI_concat(morphine_instance_t I, struct vector *a, struct vector *b) {
    ml_size size =
        mm_overflow_opc_add(a->size.accessible, b->size.accessible, throwI_error(I, "too big concat vector length"));

    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(a));
    gcI_safe(I, valueI_object(b));
    struct vector *result = gcI_safe_obj(I, vector, vectorI_create(I, size, a->dynamic));

    memcpy(result->values, a->values, ((size_t) a->size.accessible) * sizeof(struct value));
    memcpy(result->values + a->size.accessible, b->values, ((size_t) b->size.accessible) * sizeof(struct value));

    for (ml_size i = 0; i < size; i++) {
        gcI_valbarrier(I, result, result->values[i]);
    }

    gcI_safe_exit(I);

    return result;
}
