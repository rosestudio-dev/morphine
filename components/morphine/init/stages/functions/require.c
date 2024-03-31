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

void require(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs(S, 2, "string", "string,string");

            mapi_push_arg(S, 0);
            const char *id = mapi_get_string(S);

            bool has = mapi_registry_get(S);

            if (!has) {
                mapi_pop(S, 1);

                struct require_loader *result = search(table, id);
                if (result == NULL && mapi_instance(S)->require_loader_table != NULL) {
                    result = search(mapi_instance(S)->require_loader_table, id);
                }

                if (result == NULL) {
                    mapi_errorf(S, "Cannot find '%s' library", id);
                }

                result->loader(S);
                mapi_push_arg(S, 0);
                mapi_peek(S, 1);
                mapi_registry_set(S);
            }

            if (variant == 0) {
                nb_return();
            }

            mapi_push_arg(S, 1);
            const char *symbol = mapi_get_string(S);
            bool has_part = mapi_table_get(S);

            if (!has_part) {
                mapi_errorf(S, "Cannot find '%s' symbol from '%s' library", symbol, id);
            }

            nb_return();
    nb_end
}
