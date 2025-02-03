//
// Created by whyiskra on 25.12.23.
//

#include <string.h>
#include "morphine/auxiliary/expect.h"
#include "morphine/api.h"

MORPHINE_AUX void maux_expect(morphine_coroutine_t U, const char *type) {
    if (!mapi_is(U, type)) {
        mapi_errorf(U, "expected %s, but got %s", type, mapi_type(U));
    }
}

MORPHINE_AUX void maux_expect_args(morphine_coroutine_t U, ml_size count) {
    ml_size got = mapi_args(U);
    if (got != count) {
        if (count == 0) {
            mapi_errorf(U, "didn't expect arguments, but got %"MLIMIT_SIZE_PR, got);
        } else if (count == 1) {
            mapi_errorf(U, "expected 1 argument, but got %"MLIMIT_SIZE_PR, got);
        } else {
            mapi_errorf(U, "expected %"MLIMIT_SIZE_PR" arguments, but got %"MLIMIT_SIZE_PR, count, got);
        }
    }
}

MORPHINE_AUX void maux_expect_args_minimum(morphine_coroutine_t U, ml_size count) {
    ml_size got = mapi_args(U);
    if (got < count) {
        if (count == 1) {
            mapi_errorf(U, "expected minimum 1 argument, but got %"MLIMIT_SIZE_PR, got);
        } else {
            mapi_errorf(U, "expected minimum %"MLIMIT_SIZE_PR" arguments, but got %"MLIMIT_SIZE_PR, count, got);
        }
    }
}
