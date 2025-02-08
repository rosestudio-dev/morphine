//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/core/convert.h"
#include "morphine/object/coroutine.h"

MORPHINE_API void mapi_push_nil(morphine_coroutine_t U) {
    stackI_push(U, valueI_nil);
}

MORPHINE_API void mapi_push_integer(morphine_coroutine_t U, ml_integer value) {
    stackI_push(U, valueI_integer(value));
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
    struct value result = valueI_integer(convertI_to_integer(U->I, value));
    stackI_replace(U, 0, result);
}

MORPHINE_API void mapi_to_based_integer(morphine_coroutine_t U, ml_size base) {
    struct value value = stackI_peek(U, 0);
    struct value result = valueI_integer(convertI_to_basedinteger(U->I, value, base));
    stackI_replace(U, 0, result);
}

MORPHINE_API void mapi_to_decimal(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct value result = valueI_decimal(convertI_to_decimal(U->I, value));
    stackI_replace(U, 0, result);
}

MORPHINE_API void mapi_to_boolean(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct value result = valueI_boolean(valueI_tobool(value));
    stackI_replace(U, 0, result);
}

MORPHINE_API void mapi_to_string(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct value result = valueI_object(convertI_to_string(U->I, value));
    stackI_replace(U, 0, result);
}

MORPHINE_API ml_hash mapi_hash(morphine_coroutine_t U) {
    return valueI_hash(stackI_peek(U, 0));
}

MORPHINE_API int mapi_compare(morphine_coroutine_t U) {
    struct value a = stackI_peek(U, 1);
    struct value b = stackI_peek(U, 0);
    return valueI_compare(a, b);
}

MORPHINE_API void mapi_push_csize(morphine_coroutine_t U, size_t value, const char *name) {
    if (name == NULL) {
        name = "size";
    }

    ml_integer integer =
        mm_overflow_opc_ucast(ml_integer, value, throwI_errorf(U->I, "cannot convert %s to integer", name));

    stackI_push(U, valueI_integer(integer));
}

MORPHINE_API ml_size mapi_get_size(morphine_coroutine_t U, const char *name) {
    if (name == NULL) {
        name = "size";
    }

    ml_integer integer = valueI_as_integer_or_error(U->I, stackI_peek(U, 0));
    return mm_overflow_opc_ucast(ml_size, integer, throwI_errorf(U->I, "cannot convert integer to %s", name));
}

MORPHINE_API size_t mapi_get_csize(morphine_coroutine_t U, const char *name) {
    if (name == NULL) {
        name = "size";
    }

    ml_integer integer = valueI_as_integer_or_error(U->I, stackI_peek(U, 0));
    return mm_overflow_opc_ucast(size_t, integer, throwI_errorf(U->I, "cannot convert integer to %s", name));
}

MORPHINE_API ml_size mapi_csize2size(morphine_coroutine_t U, size_t value, const char *name) {
    if (name == NULL) {
        name = "size";
    }

    return mm_overflow_opc_ucast(ml_size, value, throwI_errorf(U->I, "cannot convert csize to %s", name));
}
