//
// Created by why-iskra on 30.03.2024.
//

#include <string.h>
#include "morphine/libs/require.h"
#include "morphine/api.h"
#include "morphine/core/instance.h"
#include "morphine/libs/loader.h"
#include "morphine/auxiliary.h"

static morphine_require_entry_t table[] = {
    { "base",      mlib_base_loader },
    { "gc",        mlib_gc_loader },
    { "coroutine", mlib_coroutine_loader },
    { "math",      mlib_math_loader },
    { "string",    mlib_string_loader },
    { "table",     mlib_table_loader },
    { "userdata",  mlib_userdata_loader },
    { "vector",    mlib_vector_loader },
    { "value",     mlib_value_loader },
    { "registry",  mlib_registry_loader },
    { "sio",       mlib_sio_loader },
    { "bitwise",   mlib_bitwise_loader },
    { NULL, NULL }
};

static inline morphine_require_entry_t *search(morphine_require_entry_t *entries, const char *str_id) {
    morphine_require_entry_t *current = entries;
    while (current->name != NULL) {
        if (strcmp(str_id, current->name) == 0) {
            return current;
        }

        current++;
    }

    return NULL;
}

MORPHINE_LIB void mlib_require(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            bool force = false;
            if(mapi_args(U) == 2) {
                mapi_push_arg(U, 1);
                force = mapi_get_boolean(U);
            } else {
                maux_expect_args(U, 1);
            }

            mapi_push_arg(U, 0);
            const char *id = mapi_get_string(U);

            bool has = mapi_registry_get(U);

            if (!has | force) {
                mapi_pop(U, 1);

                morphine_require_entry_t *result = search(table, id);
                if (result == NULL && mapi_instance(U)->require_table != NULL) {
                    result = search(mapi_instance(U)->require_table, id);
                }

                if (result == NULL) {
                    mapi_errorf(U, "Cannot find '%s' library", id);
                }

                result->loader(U);
                mapi_push_arg(U, 0);
                mapi_peek(U, 1);
                mapi_registry_set(U);
            }
            nb_return();
    nb_end
}
