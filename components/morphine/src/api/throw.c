//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/string.h"
#include "morphine/core/throw.h"
#include "morphine/core/instance.h"

MORPHINE_API morphine_noret void mapi_errorf(morphine_coroutine_t U, const char *str, ...) {
    va_list args;
    va_start(args, str);
    struct string *result = stringI_createva(U->I, str, args);
    va_end(args);

    throwI_errorv(U->I, valueI_object(result));
}

MORPHINE_API morphine_noret void mapi_error(morphine_coroutine_t U, const char *message) {
    if (message == NULL) {
        struct value value = stackI_peek(U, 0);
        throwI_errorv(U->I, value);
    } else {
        throwI_error(U->I, message);
    }
}

MORPHINE_API morphine_noret void mapi_ierrorf(morphine_instance_t I, const char *str, ...) {
    va_list args;
    va_start(args, str);
    struct string *result = stringI_createva(I, str, args);
    va_end(args);

    throwI_errorv(I, valueI_object(result));
}

MORPHINE_API morphine_noret void mapi_ierror(morphine_instance_t I, const char *message) {
    throwI_error(I, message);
}

MORPHINE_API void mapi_catchable(morphine_coroutine_t U, size_t callstate) {
    throwI_catchable(U, callstate);
}

MORPHINE_API void mapi_push_thrown(morphine_coroutine_t U) {
    stackI_push(U, U->thrown);
    U->thrown = valueI_nil;
}

MORPHINE_API const char *mapi_signal_message(morphine_instance_t I) {
    if (I == NULL) {
        return "initial error";
    }

    return throwI_message(I);
}

MORPHINE_API bool mapi_is_nested_signal(morphine_instance_t I) {
    if (I == NULL) {
        return false;
    }

    return throwI_is_nested_signal(I);
}
