//
// Created by why-iskra on 30.03.2024.
//

#include <string.h>
#include "../functions.h"
#include "morphine/core/instance.h"
#include "morphine/libs/loader.h"
#include "morphine/api.h"
#include "morphine/auxiliary.h"

static struct require_loader table[] = {
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
    { NULL, NULL }
};

static inline struct require_loader *search(struct require_loader *loader, const char *str_id) {
    struct require_loader *current = loader;
    while (current->name != NULL) {
        if (strcmp(str_id, current->name) == 0) {
            return current;
        }

        current++;
    }

    return NULL;
}

void require(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t args = mapi_args(U);

            mapi_push_arg(U, 0);
            maux_expect(U, "string");
            const char *id = mapi_get_string(U);

            bool has = mapi_registry_get(U);
            bool force = false;
            if (args > 1) {
                maux_expect_args(U, 2);
                mapi_push_arg(U, 1);
                maux_expect(U, "boolean");
                force = mapi_get_boolean(U);
            }

            if (!has | force) {
                mapi_pop(U, 1);

                struct require_loader *result = search(table, id);
                if (result == NULL && mapi_instance(U)->require_loader_table != NULL) {
                    result = search(mapi_instance(U)->require_loader_table, id);
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
