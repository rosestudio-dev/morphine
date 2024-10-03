//
// Created by whyiskra on 25.12.23.
//

#include <string.h>
#include "morphine/api.h"
#include "morphine/core/value.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/string.h"
#include "morphine/core/throw.h"

MORPHINE_API void mapi_push_nil(morphine_coroutine_t U) {
    stackI_push(U, valueI_nil);
}

MORPHINE_API void mapi_push_integer(morphine_coroutine_t U, ml_integer value) {
    stackI_push(U, valueI_integer(value));
}

MORPHINE_API void mapi_push_size(morphine_coroutine_t U, size_t value, const char *name) {
    ml_size size;
    if (name == NULL) {
        size = valueI_csize2size(U->I, value);
    } else {
        size = valueI_csize2namedsize(U->I, value, name);
    }

    stackI_push(U, valueI_size(size));
}

MORPHINE_API void mapi_push_decimal(morphine_coroutine_t U, ml_decimal value) {
    stackI_push(U, valueI_decimal(value));
}

MORPHINE_API void mapi_push_boolean(morphine_coroutine_t U, bool value) {
    stackI_push(U, valueI_boolean(value));
}

MORPHINE_API void mapi_push_raw(morphine_coroutine_t U, void *value) {
    stackI_push(U, valueI_raw(value));
}

MORPHINE_API ml_integer mapi_get_integer(morphine_coroutine_t U) {
    return valueI_as_integer_or_error(U->I, stackI_peek(U, 0));
}

MORPHINE_API ml_size mapi_get_size(morphine_coroutine_t U, const char *name) {
    ml_integer integer = valueI_as_integer_or_error(U->I, stackI_peek(U, 0));
    if (name == NULL) {
        return valueI_integer2size(U->I, integer);
    } else {
        return valueI_integer2namedsize(U->I, integer, name);
    }
}

MORPHINE_API ml_decimal mapi_get_decimal(morphine_coroutine_t U) {
    return valueI_as_decimal_or_error(U->I, stackI_peek(U, 0));
}

MORPHINE_API bool mapi_get_boolean(morphine_coroutine_t U) {
    return valueI_as_boolean_or_error(U->I, stackI_peek(U, 0));
}

MORPHINE_API uintptr_t mapi_get_raw(morphine_coroutine_t U) {
    return valueI_as_raw_or_error(U->I, stackI_peek(U, 0));
}

MORPHINE_API void mapi_to_integer(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct value result = valueI_value2integer(U->I, value);
    stackI_replace(U, 0, result);
}

MORPHINE_API void mapi_to_size(morphine_coroutine_t U, const char *name) {
    struct value value = stackI_peek(U, 0);
    struct value result = valueI_value2size(U->I, value, name);
    stackI_replace(U, 0, result);
}

MORPHINE_API void mapi_to_based_integer(morphine_coroutine_t U, ml_size base) {
    struct value value = stackI_peek(U, 0);
    struct value result = valueI_value2basedinteger(U->I, value, base);
    stackI_replace(U, 0, result);
}

MORPHINE_API void mapi_to_based_size(morphine_coroutine_t U, ml_size base, const char *name) {
    struct value value = stackI_peek(U, 0);
    struct value result = valueI_value2basedsize(U->I, value, base, name);
    stackI_replace(U, 0, result);
}

MORPHINE_API void mapi_to_decimal(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct value result = valueI_value2decimal(U->I, value);
    stackI_replace(U, 0, result);
}

MORPHINE_API void mapi_to_boolean(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct value result = valueI_value2boolean(U->I, value);
    stackI_replace(U, 0, result);
}

MORPHINE_API void mapi_to_string(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct value result = valueI_value2string(U->I, value);
    stackI_replace(U, 0, result);
}

MORPHINE_API ml_hash mapi_hash(morphine_coroutine_t U) {
    return valueI_hash(U->I, stackI_peek(U, 0));
}

MORPHINE_API int mapi_compare(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 1);
    struct value b = stackI_peek(U, 0);
    return valueI_compare(U->I, a, b);
}

MORPHINE_API bool mapi_equal(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 1);
    struct value b = stackI_peek(U, 0);
    return valueI_equal(U->I, a, b);
}

MORPHINE_API ml_size mapi_csize2size(morphine_coroutine_t U, size_t value, const char *name) {
    if (name == NULL) {
        return valueI_csize2size(U->I, value);
    } else {
        return valueI_csize2namedsize(U->I, value, name);
    }
}
