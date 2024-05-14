//
// Created by whyiskra on 25.12.23.
//

#include <string.h>
#include "morphine/auxiliary/expect.h"
#include "morphine/api.h"

MORPHINE_AUX void maux_expect(morphine_coroutine_t U, const char *type) {
    if (!mapi_is(U, type)) {
        mapi_errorf(U, "Expected %s, but got %s", type, mapi_type(U));
    }
}

MORPHINE_AUX void maux_expect_args(morphine_coroutine_t U, size_t count) {
    size_t got = mapi_args(U);
    if (got != count) {
        if (count == 0) {
            mapi_errorf(U, "Didn't expect arguments, but got %zu", got);
        } else if (count == 1) {
            mapi_errorf(U, "Expected 1 argument, but got %zu", got);
        } else {
            mapi_errorf(U, "Expected %zu arguments, but got %zu", count, got);
        }
    }
}
