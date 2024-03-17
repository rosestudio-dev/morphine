//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/object/string.h"
#include "morphine/object/state.h"
#include "morphine/core/throw.h"
#include <stdarg.h>

MORPHINE_API void mapi_push_string(morphine_state_t S, const char *str, ...) {
    va_list args;
    va_start(args, str);
    struct string *result = stringI_createva(S->I, str, args);
    va_end(args);

    stackI_push(S, valueI_object(result));
}

MORPHINE_API void mapi_push_stringv(morphine_state_t S, const char *str, va_list args) {
    struct string *result = stringI_createva(S->I, str, args);
    stackI_push(S, valueI_object(result));
}

MORPHINE_API const char *mapi_get_string(morphine_state_t S) {
    return valueI_as_string_or_error(S, stackI_peek(S, 0))->chars;
}
