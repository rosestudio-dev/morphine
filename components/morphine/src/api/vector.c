//
// Created by why-iskra on 22.04.2024.
//

#include "morphine/api.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/vector.h"
#include "morphine/object/string.h"
#include "morphine/core/throw.h"

MORPHINE_API void mapi_push_vector(morphine_coroutine_t U, ml_size size) {
    stackI_push(U, valueI_object(vectorI_create(U->I, size)));
}

MORPHINE_API void mapi_vector_resize(morphine_coroutine_t U, ml_size size) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    vectorI_resize(U->I, vector, size);
}

MORPHINE_API void mapi_vector_clear(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    vectorI_resize(U->I, vector, 0);
}

MORPHINE_API void mapi_vector_copy(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    stackI_push(U, valueI_object(vectorI_copy(U->I, vector)));
}

MORPHINE_API void mapi_vector_mode_mutable(morphine_coroutine_t U, bool is_mutable) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    vectorI_mode_mutable(U->I, vector, is_mutable);
}

MORPHINE_API void mapi_vector_mode_fixed(morphine_coroutine_t U, bool is_fixed) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    vectorI_mode_fixed(U->I, vector, is_fixed);
}

MORPHINE_API void mapi_vector_mode_lock(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    vectorI_mode_lock(U->I, vector);
}

MORPHINE_API bool mapi_vector_mode_is_mutable(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    return vector->mode.mutable;
}

MORPHINE_API bool mapi_vector_mode_is_fixed(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    return vector->mode.fixed;
}

MORPHINE_API bool mapi_vector_mode_is_locked(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    return vector->mode.locked;
}

MORPHINE_API void mapi_vector_set(morphine_coroutine_t U, ml_size index) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 1));
    struct value value = stackI_peek(U, 0);
    vectorI_set(U->I, vector, index, value);
    stackI_pop(U, 1);
}

MORPHINE_API void mapi_vector_get(morphine_coroutine_t U, ml_size index) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    stackI_push(U, vectorI_get(U->I, vector, index));
}

MORPHINE_API void mapi_vector_add(morphine_coroutine_t U, ml_size index) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 1));
    struct value value = stackI_peek(U, 0);
    vectorI_add(U->I, vector, index, value);
    stackI_pop(U, 1);
}

MORPHINE_API void mapi_vector_remove(morphine_coroutine_t U, ml_size index) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    stackI_push(U, vectorI_remove(U->I, vector, index));
}

MORPHINE_API void mapi_vector_push(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 1));
    struct value value = stackI_peek(U, 0);
    vectorI_add(U->I, vector, vector->size.accessible, value);
    stackI_pop(U, 1);
}

MORPHINE_API void mapi_vector_pop(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    if (vector->size.accessible == 0) {
        throwI_error(U->I, "cannot pop value from empty vector");
    }

    stackI_push(U, vectorI_remove(U->I, vector, vector->size.accessible - 1));
}

MORPHINE_API void mapi_vector_peek(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    if (vector->size.accessible == 0) {
        throwI_error(U->I, "cannot peek value from empty vector");
    }

    stackI_push(U, vectorI_get(U->I, vector, vector->size.accessible - 1));
}

MORPHINE_API void mapi_vector_push_front(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 1));
    struct value value = stackI_peek(U, 0);
    vectorI_add(U->I, vector, 0, value);
    stackI_pop(U, 1);
}

MORPHINE_API void mapi_vector_pop_front(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    if (vector->size.accessible == 0) {
        throwI_error(U->I, "cannot pop value from front of empty vector");
    }

    stackI_push(U, vectorI_remove(U->I, vector, 0));
}

MORPHINE_API void mapi_vector_peek_front(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    if (vector->size.accessible == 0) {
        throwI_error(U->I, "cannot peek value from front of empty vector");
    }

    stackI_push(U, vectorI_get(U->I, vector, 0));
}

MORPHINE_API ml_size mapi_vector_len(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    return vector->size.accessible;
}
