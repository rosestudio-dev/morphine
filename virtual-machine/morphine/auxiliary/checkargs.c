//
// Created by whyiskra on 25.12.23.
//

#include <stdarg.h>
#include <string.h>
#include "morphine/auxiliary.h"
#include "morphine/api.h"

MORPHINE_AUX void maux_checkargs_fixed(morphine_state_t S, size_t count) {
    size_t cargs = mapi_args(S);

    if (cargs != count) {
        mapi_push_string(S, "Expected %d, but got %d", count, cargs);
        mapi_error(S);
    }
}

MORPHINE_AUX void maux_checkargs_pattern(morphine_state_t S, size_t count, ...) {
    maux_checkargs_fixed(S, count);

    va_list vargs;
    va_start(vargs, count);
    for (size_t i = 0; i < count; i++) {
        const char *type_name = va_arg(vargs, const char *);

        if (strcmp(type_name, "*") == 0) {
            continue;
        }

        mapi_push_arg(S, i);
        const char *arg_type = mapi_type(S);

        if (strcmp(type_name, "callable") == 0) {
            if (!mapi_is_callable(S)) {
                va_end(vargs);
                mapi_errorf(S, "Expected callable, but got %s", type_name, arg_type);
            }
        } else {
            if (strcmp(type_name, arg_type) == 0) {
                va_end(vargs);
                mapi_errorf(S, "Expected %s, but got %s", type_name, arg_type);
            }
        }

        mapi_pop(S, 1);
    }
    va_end(vargs);
}

MORPHINE_AUX void maux_checkargs_self(morphine_state_t S, size_t count) {
    if (count == 0) {
        mapi_errorf(S, "Self checkargs variant requires count more than zero");
    }

    size_t cargs = maux_checkargs_or(S, count - 1, count);

    if (cargs == count) {
        mapi_push_arg(S, 0);
    } else {
        mapi_push_self(S);
    }
}

MORPHINE_AUX size_t maux_checkargs_or(morphine_state_t S, size_t count1, size_t count2) {
    size_t cargs = mapi_args(S);

    if (cargs != count1 && cargs != count2) {
        mapi_push_string(S, "Expected %d or %d, but got %d", count1, count2, cargs);
        mapi_error(S);
    }

    return cargs;
}

MORPHINE_AUX size_t maux_checkargs_minimum(morphine_state_t S, size_t minimum) {
    size_t cargs = mapi_args(S);

    if (minimum > cargs) {
        mapi_push_string(S, "Expected minimum %d, but got %d", minimum, cargs);
        mapi_error(S);
    }

    return cargs;
}
