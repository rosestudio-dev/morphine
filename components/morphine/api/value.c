//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/core/value.h"
#include "morphine/object/state.h"
#include "morphine/object/string.h"
#include "morphine/core/throw.h"
#include "morphine/stack/access.h"
#include "morphine/stack/call.h"

MORPHINE_API void mapi_push_nil(morphine_state_t S) {
    stackI_push(S, valueI_nil);
}

MORPHINE_API void mapi_push_integer(morphine_state_t S, ml_integer value) {
    stackI_push(S, valueI_integer(value));
}

MORPHINE_API void mapi_push_size(morphine_state_t S, size_t value) {
    stackI_push(S, valueI_size2integer(S->I, value));
}

MORPHINE_API void mapi_push_decimal(morphine_state_t S, ml_decimal value) {
    stackI_push(S, valueI_decimal(value));
}

MORPHINE_API void mapi_push_boolean(morphine_state_t S, bool value) {
    stackI_push(S, valueI_boolean(value));
}

MORPHINE_API void mapi_push_raw(morphine_state_t S, void *value) {
    stackI_push(S, valueI_raw(value));
}

MORPHINE_API ml_integer mapi_get_integer(morphine_state_t S) {
    return valueI_as_integer_or_error(S->I, stackI_peek(S, 0));
}

MORPHINE_API size_t mapi_get_size(morphine_state_t S) {
    ml_integer integer = valueI_as_integer_or_error(S->I, stackI_peek(S, 0));
    return valueI_integer2size(S->I, integer);
}

MORPHINE_API ml_decimal mapi_get_decimal(morphine_state_t S) {
    return valueI_as_decimal_or_error(S->I, stackI_peek(S, 0));
}

MORPHINE_API bool mapi_get_boolean(morphine_state_t S) {
    return valueI_as_boolean_or_error(S->I, stackI_peek(S, 0));
}

MORPHINE_API void *mapi_get_raw(morphine_state_t S) {
    return valueI_as_raw_or_error(S->I, stackI_peek(S, 0));
}

MORPHINE_API const char *mapi_type(morphine_state_t S) {
    return valueI_type2string(S->I, stackI_peek(S, 0).type);
}

MORPHINE_API bool mapi_checktype(morphine_state_t S, const char *name) {
    return valueI_string2type(S->I, name) == stackI_peek(S, 0).type;
}

MORPHINE_API void mapi_to_integer(morphine_state_t S) {
    struct value *value = stackI_vector(S, 0, 1);
    *value = valueI_value2integer(S->I, *value);
}

MORPHINE_API void mapi_to_decimal(morphine_state_t S) {
    struct value *value = stackI_vector(S, 0, 1);
    *value = valueI_value2decimal(S->I, *value);
}

MORPHINE_API void mapi_to_boolean(morphine_state_t S) {
    struct value *value = stackI_vector(S, 0, 1);
    *value = valueI_value2boolean(S->I, *value);
}

MORPHINE_API bool mapi_is_callable(morphine_state_t S) {
    struct value value = stackI_peek(S, 0);
    return !valueI_is_nil(callstackI_extract_callable(S->I, value));
}

MORPHINE_API void mapi_to_string(morphine_state_t S) {
    struct value *value = stackI_vector(S, 0, 1);
    *value = valueI_value2string(S->I, *value);
}
