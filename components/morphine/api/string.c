//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/object/string.h"
#include "morphine/object/state.h"
#include "morphine/core/throw.h"
#include "morphine/stack/access.h"
#include <stdarg.h>
#include <string.h>

MORPHINE_API void mapi_push_stringf(morphine_state_t S, const char *str, ...) {
    va_list args;
    va_start(args, str);
    struct string *result = stringI_createva(S->I, str, args);
    va_end(args);

    stackI_push(S, valueI_object(result));
}

MORPHINE_API void mapi_push_string(morphine_state_t S, const char *str) {
    struct string *result = stringI_create(S->I, str);
    stackI_push(S, valueI_object(result));
}

MORPHINE_API void mapi_push_stringn(morphine_state_t S, const char *str, size_t size) {
    char *buffer;
    struct string *result = stringI_createn(S->I, size, &buffer);
    memcpy(buffer, str, size);

    stackI_push(S, valueI_object(result));
}

MORPHINE_API void mapi_push_stringv(morphine_state_t S, const char *str, va_list args) {
    struct string *result = stringI_createva(S->I, str, args);
    stackI_push(S, valueI_object(result));
}

MORPHINE_API const char *mapi_get_string(morphine_state_t S) {
    return valueI_as_string_or_error(S, stackI_peek(S, 0))->chars;
}

MORPHINE_API size_t mapi_string_len(morphine_state_t S) {
    return valueI_as_string_or_error(S, stackI_peek(S, 0))->size;
}

MORPHINE_API void mapi_string_concat(morphine_state_t S) {
    struct string *a = valueI_as_string_or_error(S, stackI_peek(S, 1));
    struct string *b = valueI_as_string_or_error(S, stackI_peek(S, 0));
    struct string *result = stringI_concat(S->I, a, b);
    stackI_pop(S, 1);
    *stackI_vector(S, 0, 1) = valueI_object(result);
}
