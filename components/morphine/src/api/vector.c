//
// Created by why-iskra on 22.04.2024.
//

#include "morphine/api.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/vector.h"
#include "morphine/object/string.h"
#include "morphine/core/throw.h"

MORPHINE_API void mapi_push_vector_fix(morphine_coroutine_t U, ml_size size) {
    stackI_push(U, valueI_object(vectorI_create(U->I, size, false)));
}

MORPHINE_API void mapi_push_vector_dyn(morphine_coroutine_t U, ml_size size) {
    stackI_push(U, valueI_object(vectorI_create(U->I, size, true)));
}

MORPHINE_API void mapi_vector_resize(morphine_coroutine_t U, ml_size size) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    vectorI_resize(U->I, vector, size);
}

MORPHINE_API void mapi_vector_copy(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    stackI_push(U, valueI_object(vectorI_copy(U->I, vector)));
}

MORPHINE_API void mapi_vector_concat(morphine_coroutine_t U) {
    struct vector *a = valueI_as_vector_or_error(U->I, stackI_peek(U, 1));
    struct vector *b = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    struct vector *result = vectorI_concat(U->I, a, b);
    stackI_replace(U, 1, valueI_object(result));
    stackI_pop(U, 1);
}

MORPHINE_API void mapi_vector_set(morphine_coroutine_t U, ml_size index) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 1));
    struct value value = stackI_peek(U, 0);
    vectorI_set(U->I, vector, index, value);
    stackI_pop(U, 1);
}

MORPHINE_API void mapi_vector_add(morphine_coroutine_t U, ml_size index) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 1));
    struct value value = stackI_peek(U, 0);
    vectorI_add(U->I, vector, index, value);
    stackI_pop(U, 1);
}

MORPHINE_API bool mapi_vector_has(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 1));
    struct value value = stackI_peek(U, 0);
    return vectorI_has(vector, value);
}

MORPHINE_API void mapi_vector_get(morphine_coroutine_t U, ml_size index) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    stackI_push(U, vectorI_get(U->I, vector, index));
}

MORPHINE_API void mapi_vector_remove(morphine_coroutine_t U, ml_size index) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    stackI_push(U, vectorI_remove(U->I, vector, index));
}

MORPHINE_API ml_size mapi_vector_len(morphine_coroutine_t U) {
    struct vector *vector = valueI_as_vector_or_error(U->I, stackI_peek(U, 0));
    return vector->size.accessible;
}
