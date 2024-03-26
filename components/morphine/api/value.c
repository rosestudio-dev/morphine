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

MORPHINE_API void mapi_push_integer(morphine_state_t S, morphine_integer_t value) {
    stackI_push(S, valueI_integer(value));
}

MORPHINE_API void mapi_push_size(morphine_state_t S, size_t value) {
    stackI_push(S, valueI_size2integer(S, value));
}

MORPHINE_API void mapi_push_decimal(morphine_state_t S, morphine_decimal_t value) {
    stackI_push(S, valueI_decimal(value));
}

MORPHINE_API void mapi_push_boolean(morphine_state_t S, bool value) {
    stackI_push(S, valueI_boolean(value));
}

MORPHINE_API void mapi_push_raw(morphine_state_t S, void *value) {
    stackI_push(S, valueI_raw(value));
}

MORPHINE_API morphine_integer_t mapi_get_integer(morphine_state_t S) {
    return valueI_as_integer_or_error(S, stackI_peek(S, 0));
}

MORPHINE_API size_t mapi_get_size(morphine_state_t S) {
    morphine_integer_t integer = valueI_as_integer_or_error(S, stackI_peek(S, 0));
    return valueI_integer2size(S, integer);
}

MORPHINE_API morphine_decimal_t mapi_get_decimal(morphine_state_t S) {
    return valueI_as_decimal_or_error(S, stackI_peek(S, 0));
}

MORPHINE_API bool mapi_get_boolean(morphine_state_t S) {
    return valueI_as_boolean_or_error(S, stackI_peek(S, 0));
}

MORPHINE_API void *mapi_get_raw(morphine_state_t S) {
    return valueI_as_raw_or_error(S, stackI_peek(S, 0));
}

MORPHINE_API const char *mapi_type(morphine_state_t S) {
    return valueI_type2string(S->I, stackI_peek(S, 0).type);
}

MORPHINE_API bool mapi_checktype(morphine_state_t S, const char *name) {
    return valueI_string2type(S, name) == stackI_peek(S, 0).type;
}

MORPHINE_API void mapi_to_integer(morphine_state_t S) {
    struct value *value = stackI_vector(S, 0, 1);
    *value = valueI_value2integer(S, *value);
}

MORPHINE_API void mapi_to_based_integer(morphine_state_t S, uint8_t base) {
    struct value *value = stackI_vector(S, 0, 1);
    struct string *string = valueI_as_string_or_error(S, *value);
    morphine_integer_t result;

    if (!platformI_string2integer(string->chars, &result, base)) {
        throwI_errorf(S, "Cannot convert string '%s' to integer with base %d", string->chars, base);
    }

    *value = valueI_integer(result);
}

MORPHINE_API void mapi_to_decimal(morphine_state_t S) {
    struct value *value = stackI_vector(S, 0, 1);
    *value = valueI_value2decimal(S, *value);
}

MORPHINE_API void mapi_to_boolean(morphine_state_t S) {
    struct value *value = stackI_vector(S, 0, 1);
    *value = valueI_value2boolean(S, *value);
}

MORPHINE_API bool mapi_is_callable(morphine_state_t S) {
    struct value value = stackI_peek(S, 0);
    return !valueI_is_nil(callstackI_extract_callable(S->I, value));
}

MORPHINE_API void mapi_to_string(morphine_state_t S) {
    struct value *value = stackI_vector(S, 0, 1);
    *value = valueI_value2string(S->I, *value);
}
