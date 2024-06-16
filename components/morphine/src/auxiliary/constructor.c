//
// Created by whyiskra on 25.12.23.
//

#include <string.h>
#include "morphine/auxiliary/constructor.h"
#include "morphine/api.h"

MORPHINE_AUX void maux_construct(
    morphine_coroutine_t U,
    struct maux_construct_field *table,
    const char *prefix
) {
    void *registry_key = table;

    mapi_push_table(U);

    while (table->name != NULL) {
        mapi_push_string(U, table->name);

        const char *name = table->name;
        if (prefix != NULL) {
            mapi_peek(U, 0);
            mapi_push_string(U, prefix);
            mapi_rotate(U, 2);
            mapi_string_concat(U);
            name = mapi_get_string(U);
        }

        mapi_push_native(U, name, table->value);

        if (prefix != NULL) {
            mapi_rotate(U, 2);
            mapi_pop(U, 1);
        }

        mapi_push_raw(U, registry_key);
        mapi_registry_set_key(U);

        mapi_table_set(U);

        table++;
    }
}

MORPHINE_AUX void maux_construct_call(
    morphine_coroutine_t U,
    struct maux_construct_field *table,
    const char *prefix,
    const char *name,
    ml_size argc
) {
    void *registry_key = table;

    while (table->name != NULL) {
        if (strcmp(name, table->name) == 0) {
            goto found;
        }

        table++;
    }

    mapi_errorf(U, "function %s wasn't found in construct table", name);

found:;
    const char *native_name = table->name;
    if (prefix != NULL) {
        mapi_push_string(U, prefix);
        mapi_push_string(U, native_name);
        mapi_string_concat(U);
        native_name = mapi_get_string(U);
    }

    mapi_push_native(U, native_name, table->value);

    if (prefix != NULL) {
        mapi_rotate(U, 2);
        mapi_pop(U, 1);
    }

    mapi_push_raw(U, registry_key);
    mapi_registry_set_key(U);

    mapi_calli(U, argc);
}
