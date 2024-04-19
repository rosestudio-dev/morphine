//
// Created by whyiskra on 25.12.23.
//

#include <string.h>
#include "morphine/auxiliary/constructor.h"
#include "morphine/api.h"

MORPHINE_AUX void maux_construct(morphine_coroutine_t U, struct maux_construct_field *table) {
    void *registry_key = table;

    mapi_push_table(U);

    while (table->name != NULL) {
        mapi_push_stringf(U, table->name);
        mapi_push_native(U, table->name, table->value);

        mapi_push_raw(U, registry_key);
        mapi_registry_set_key(U);

        mapi_table_set(U);

        table++;
    }
}

MORPHINE_AUX void maux_construct_call(
    morphine_coroutine_t U,
    struct maux_construct_field *table,
    const char *name,
    size_t argc
) {
    void *registry_key = table;

    while (table->name != NULL) {
        if (strcmp(name, table->name) == 0) {
            goto found;
        }

        table++;
    }

    mapi_errorf(U, "Function %s wasn't found in construct table", name);

found:;
    mapi_push_native(U, table->name, table->value);
    mapi_push_raw(U, registry_key);
    mapi_registry_set_key(U);

    mapi_calli(U, argc);
}
