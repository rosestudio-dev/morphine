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
    if (vector == NULL) {
        throwI_error(I, "vector is null");
    }

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
    if (vector == NULL) {
        throwI_error(I, "vector is null");
    }

    if (vector->lock.mode) {
        throwI_error(I, "vector is locked");
    }

    vector->mode.mutable = is_mutable;
}

void vectorI_mode_accessible(morphine_instance_t I, struct vector *vector, bool is_accessible) {
    if (vector == NULL) {
        throwI_error(I, "vector is null");
    }

    if (vector->lock.mode) {
        throwI_error(I, "vector is locked");
    }

    vector->mode.accessible = is_accessible;
}

void vectorI_lock_mode(morphine_instance_t I, struct vector *vector) {
    if (vector == NULL) {
        throwI_error(I, "vector is null");
    }

    vector->lock.mode = true;
}

ml_size vectorI_size(morphine_instance_t I, struct vector *vector) {
    if (vector == NULL) {
        throwI_error(I, "vector is null");
    }

    return vector->size.accessible;
}

void vectorI_set(morphine_instance_t I, struct vector *vector, ml_size index, struct value value) {
    if (vector == NULL) {
        throwI_error(I, "vector is null");
    }

    if (!vector->mode.mutable) {
        throwI_error(I, "vector is immutable");
    }

    if (index >= vector->size.accessible) {
        throwI_error(I, "vector index out of bounce");
    }

    vector->values[index] = gcI_valbarrier(I, vector, value);
}

void vectorI_add(morphine_instance_t I, struct vector *vector, ml_size index, struct value value) {
    if (vector == NULL) {
        throwI_error(I, "vector is null");
    }

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

bool vectorI_has(morphine_instance_t I, struct vector *vector, struct value value) {
    if (vector == NULL) {
        throwI_error(I, "vector is null");
    }

    for (ml_size i = 0; i < vector->size.accessible; i++) {
        if (valueI_equal(I, vector->values[i], value)) {
            return true;
        }
    }

    return false;
}

struct value vectorI_get(morphine_instance_t I, struct vector *vector, ml_size index) {
    if (vector == NULL) {
        throwI_error(I, "vector is null");
    }

    if (!vector->mode.accessible) {
        throwI_error(I, "vector is inaccessible");
    }

    if (index >= vector->size.accessible) {
        throwI_error(I, "vector index out of bounce");
    }

    return vector->values[index];
}

struct value vectorI_remove(morphine_instance_t I, struct vector *vector, ml_size index) {
    if (vector == NULL) {
        throwI_error(I, "vector is null");
    }

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
    if (vector == NULL) {
        throwI_error(I, "vector is null");
    }

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
    if (vector == NULL) {
        throwI_error(I, "vector is null");
    }

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
    if (a == NULL || b == NULL) {
        throwI_error(I, "vector is null");
    }

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

static inline void insertion_sort(morphine_instance_t I, struct value *values, ml_size n) {
    for (ml_size i = 1; i < n; i++) {
        struct value current = values[i];

        ml_size j = i;
        for (; j != 0 && valueI_compare(I, current, values[j - 1]) < 0; j--) {
            values[j] = values[j - 1];
        }

        if (j != i) {
            values[j] = current;
        }
    }
}

static inline void merge_sort(
    morphine_instance_t I,
    struct value *values,
    ml_size left,
    ml_size middle,
    ml_size right,
    struct value *temp_left,
    struct value *temp_right
) {
    ml_size len_left = middle - left + 1;
    ml_size len_right = right - middle;

    memcpy(temp_left, values + left, ((size_t) len_left) * sizeof(struct value));
    memcpy(temp_right, values + middle + 1, ((size_t) len_right) * sizeof(struct value));

    ml_size i = 0;
    ml_size j = 0;
    ml_size k = left;

    while (i < len_left && j < len_right) {
        if (valueI_compare(I, temp_left[i], temp_right[j]) <= 0) {
            values[k] = temp_left[i];
            i++;
        } else {
            values[k] = temp_right[j];
            j++;
        }
        k++;
    }

    while (i < len_left) {
        values[k] = temp_left[i];
        k++;
        i++;
    }

    while (j < len_right) {
        values[k] = temp_right[j];
        k++;
        j++;
    }
}

static inline ml_size minrun_length(ml_size n) {
    ml_size r = 0;
    while (n >= 32) {
        r |= (n & 1);
        n >>= 1;
    }
    return n + r;
}

void vectorI_sort(morphine_instance_t I, struct vector *vector) {
    if (vector == NULL) {
        throwI_error(I, "vector is null");
    }

    if (!vector->mode.mutable) {
        throwI_error(I, "vector is immutable");
    }

    ml_size vec_size = vector->size.accessible;

    if (vec_size == 0 || vec_size == 1) {
        return;
    }

    ml_size run = minrun_length(vec_size);

    {
        for (ml_size i = 0; i < vec_size / run; i++) {
            insertion_sort(I, vector->values + (i * run), run);
        }
        ml_size offset = vec_size - (vec_size % run);
        insertion_sort(I, vector->values + offset, vec_size % run);
    }

    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(vector));
    ml_size mul2size = mm_overflow_opc_mul(vec_size, 2, throwI_error(I, "sort vector too big"));
    struct userdata *userdata =
        gcI_safe_obj(I, userdata, userdataI_create_vec(I, mul2size, sizeof(struct value), NULL));

    for (ml_size factor = 0; factor < (sizeof(factor) * 8); factor++) {
        ml_size mul = ((ml_size) 1) << factor;
        if (mm_overflow_cond_mul(run, mul) || run * mul >= vec_size) {
            break;
        }

        ml_size size = run * mul;
        ml_size left = 0;
        while (left < vec_size) {
            ml_size middle = left + size - 1;
            ml_size right = min(left + (size * 2) - 1, vec_size - 1);

            if (middle < right) {
                merge_sort(
                    I,
                    vector->values,
                    left,
                    middle,
                    right,
                    (struct value *) userdata->data,
                    ((struct value *) userdata->data) + vec_size
                );
            }

            left = mm_overflow_opc_add(left, mm_overflow_opc_mul(size, 2, break), break);
        }
    }

    gcI_safe_exit(I);
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
        vectorI_lock_mode(I, vector);
    }
}
