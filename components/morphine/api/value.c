//
// Created by whyiskra on 25.12.23.
//

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

MORPHINE_API void mapi_push_size(morphine_coroutine_t U, size_t value) {
    stackI_push(U, valueI_size2integer(U->I, value));
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

MORPHINE_API ml_size mapi_get_size(morphine_coroutine_t U) {
    ml_integer integer = valueI_as_integer_or_error(U->I, stackI_peek(U, 0));
    return valueI_integer2size(U->I, integer);
}

MORPHINE_API ml_decimal mapi_get_decimal(morphine_coroutine_t U) {
    return valueI_as_decimal_or_error(U->I, stackI_peek(U, 0));
}

MORPHINE_API bool mapi_get_boolean(morphine_coroutine_t U) {
    return valueI_as_boolean_or_error(U->I, stackI_peek(U, 0));
}

MORPHINE_API void *mapi_get_raw(morphine_coroutine_t U) {
    return valueI_as_raw_or_error(U->I, stackI_peek(U, 0));
}

MORPHINE_API const char *mapi_type(morphine_coroutine_t U) {
    return valueI_type2string(U->I, stackI_peek(U, 0).type);
}

MORPHINE_API bool mapi_checktype(morphine_coroutine_t U, const char *name) {
    return valueI_string2type(U->I, name) == stackI_peek(U, 0).type;
}

MORPHINE_API void mapi_to_integer(morphine_coroutine_t U) {
    struct value *value = stackI_vector(U, 0, 1);
    *value = valueI_value2integer(U->I, *value);
}

MORPHINE_API void mapi_to_decimal(morphine_coroutine_t U) {
    struct value *value = stackI_vector(U, 0, 1);
    *value = valueI_value2decimal(U->I, *value);
}

MORPHINE_API void mapi_to_boolean(morphine_coroutine_t U) {
    struct value *value = stackI_vector(U, 0, 1);
    *value = valueI_value2boolean(U->I, *value);
}

MORPHINE_API bool mapi_is_callable(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    return !valueI_is_nil(callstackI_extract_callable(U->I, value));
}

MORPHINE_API void mapi_to_string(morphine_coroutine_t U) {
    struct value *value = stackI_vector(U, 0, 1);
    *value = valueI_value2string(U->I, *value);
}
