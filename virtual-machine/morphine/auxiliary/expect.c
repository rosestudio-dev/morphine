//
// Created by whyiskra on 1/28/24.
//

#include <string.h>
#include "morphine/auxiliary.h"
#include "morphine/api.h"

MORPHINE_AUX void maux_expect(morphine_state_t S, const char *type) {
    const char *gottype = mapi_type(S);

    if (strcmp(type, "callable") == 0) {
        if (!mapi_is_callable(S)) {
            mapi_errorf(S, "Expected callable, but got %s", gottype);
        }
    } else if (!mapi_checktype(S, type)) {
        mapi_errorf(S, "Expected %s, but got %s", type, gottype);
    }
}

