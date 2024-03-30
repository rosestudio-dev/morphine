//
// Created by whyiskra on 25.12.23.
//

#include <stdarg.h>
#include <string.h>
#include "morphine/auxiliary/args.h"
#include "morphine/api.h"

static bool check_type(
    morphine_state_t S,
    const char *expected_type,
    size_t expected_type_len,
    bool is_self,
    size_t arg
) {
    if (expected_type_len == 3 && memcmp(expected_type, "any", 3) == 0) {
        return true;
    }

    if (is_self) {
        mapi_push_self(S);
    } else {
        mapi_push_arg(S, arg);
    }

    if (expected_type_len == 8 && memcmp(expected_type, "callable", 8) == 0) {
        bool is_callable = mapi_is_callable(S);
        mapi_pop(S, 1);

        return is_callable;
    }

    const char *type = mapi_type(S);
    size_t typelen = strlen(type);

    mapi_pop(S, 1);

    if (expected_type_len != typelen || memcmp(expected_type, type, typelen) != 0) {
        return false;
    }

    return true;
}

static bool check_pattern(
    morphine_state_t S,
    const char *pattern
) {
    size_t len = strlen(pattern);

    size_t parts = 1;
    for (size_t i = 0; i < len; i++) {
        if (pattern[i] == ',') {
            parts++;
        }
    }

    size_t argc = mapi_args(S);
    size_t reqargs = 0;

    size_t start = 0;
    for (size_t i = 0; i < parts; i++) {
        const char *substr = pattern + start;
        const char *end = strchr(substr, ',');
        if (end == NULL) {
            end = pattern + len;
        }

        size_t sublen = (size_t) (end - substr);

        bool vararg = false;
        bool self = false;

        if (sublen >= 3 && memcmp(substr + (sublen - 3), "...", 3) == 0) {
            if (i < parts - 1) {
                mapi_errorf(S, "Wrong argument pattern (vararg should be always last)");
            } else {
                vararg = true;
            }
        }

        if (sublen >= 5 && memcmp(substr, "self:", 5) == 0) {
            if (vararg) {
                mapi_errorf(S, "Wrong argument pattern (vararg cannot be combine with self)");
            } else if (i > 0) {
                mapi_errorf(S, "Wrong argument pattern (self should be always first)");
            } else {
                self = true;
            }
        }

        if (vararg) {
            size_t expected_type_len = sublen - 3;

            for (size_t k = reqargs; k < argc; k++) {
                if (!check_type(S, substr, expected_type_len, false, k)) {
                    return false;
                }
            }

            break;
        }

        if (self) {
            if (!check_type(S, substr + 5, sublen - 5, true, 0)) {
                return false;
            }
        } else {
            if (reqargs >= argc) {
                return false;
            }

            if (!check_type(S, substr, sublen, false, reqargs)) {
                return false;
            }

            reqargs++;
        }

        if (i >= parts - 1 && argc != reqargs) {
            return false;
        }

        start += sublen + 1;
    }

    return true;
}

MORPHINE_AUX size_t maux_checkargs_pattern(morphine_state_t S, size_t count, ...) {
    if (count == 0) {
        if (mapi_args(S) == 0) {
            return 0;
        } else {
            mapi_errorf(S, "Arguments aren't expected");
        }
    }

    va_list vargs;
    va_start(vargs, count);

    bool found = false;
    size_t result;
    for (size_t i = 0; i < count; i++) {
        const char *pattern = va_arg(vargs, const char *);

        if (check_pattern(S, pattern)) {
            if (!found) {
                result = i;
            }
            found = true;
        }
    }

    va_end(vargs);

    if (!found) {
        mapi_errorf(S, "Unexpected arguments");
    }

    return result;
}

MORPHINE_AUX void maux_checkargs_fixed(morphine_state_t S, size_t count) {
    size_t cargs = mapi_args(S);

    if (cargs != count) {
        mapi_push_stringf(S, "Expected %d, but got %d", count, cargs);
        mapi_error(S);
    }
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
        mapi_push_stringf(S, "Expected %d or %d, but got %d", count1, count2, cargs);
        mapi_error(S);
    }

    return cargs;
}

MORPHINE_AUX size_t maux_checkargs_minimum(morphine_state_t S, size_t minimum) {
    size_t cargs = mapi_args(S);

    if (minimum > cargs) {
        mapi_push_stringf(S, "Expected minimum %d, but got %d", minimum, cargs);
        mapi_error(S);
    }

    return cargs;
}
