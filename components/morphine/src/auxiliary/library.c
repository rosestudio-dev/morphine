//
// Created by why-iskra on 18.06.2024.
//

#include "morphine/auxiliary/library.h"
#include "morphine/api.h"

MORPHINE_AUX void maux_library_get(morphine_coroutine_t U, const char *name, const char *key) {
    mapi_library(U, name, false);
    mapi_push_string(U, key);
    if (!mapi_table_get(U)) {
        mapi_errorf(U, "library doesn't contain '%s' field", key);
    }

    mapi_rotate(U, 2);
    mapi_pop(U, 1);
}
