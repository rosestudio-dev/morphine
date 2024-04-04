//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/object/string.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"
#include "morphine/stack/access.h"
#include <stdarg.h>
#include <string.h>

MORPHINE_API void mapi_push_stringf(morphine_coroutine_t U, const char *str, ...) {
    va_list args;
    va_start(args, str);
    struct string *result = stringI_createva(U->I, str, args);
    va_end(args);

    stackI_push(U, valueI_object(result));
}

MORPHINE_API void mapi_push_string(morphine_coroutine_t U, const char *str) {
    struct string *result = stringI_create(U->I, str);
    stackI_push(U, valueI_object(result));
}

MORPHINE_API void mapi_push_stringn(morphine_coroutine_t U, const char *str, size_t size) {
    char *buffer;
    struct string *result = stringI_createn(U->I, size, &buffer);
    memcpy(buffer, str, size);

    stackI_push(U, valueI_object(result));
}

MORPHINE_API void mapi_push_stringv(morphine_coroutine_t U, const char *str, va_list args) {
    struct string *result = stringI_createva(U->I, str, args);
    stackI_push(U, valueI_object(result));
}

MORPHINE_API const char *mapi_get_string(morphine_coroutine_t U) {
    return valueI_as_string_or_error(U->I, stackI_peek(U, 0))->chars;
}

MORPHINE_API size_t mapi_string_len(morphine_coroutine_t U) {
    return valueI_as_string_or_error(U->I, stackI_peek(U, 0))->size;
}

MORPHINE_API void mapi_string_concat(morphine_coroutine_t U) {
    struct string *a = valueI_as_string_or_error(U->I, stackI_peek(U, 1));
    struct string *b = valueI_as_string_or_error(U->I, stackI_peek(U, 0));
    struct string *result = stringI_concat(U->I, a, b);
    stackI_pop(U, 1);
    *stackI_vector(U, 0, 1) = valueI_object(result);
}
