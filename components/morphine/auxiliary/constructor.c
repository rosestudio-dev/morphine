//
// Created by whyiskra on 25.12.23.
//

#include <string.h>
#include "morphine/auxiliary.h"
#include "morphine/api.h"

MORPHINE_AUX void maux_construct(morphine_state_t S, struct maux_construct_field *table) {
    void *registry_key = table;

    mapi_push_table(S, 1);

    while (table->name != NULL) {
        mapi_push_stringf(S, table->name);
        mapi_push_native(S, table->name, table->value);

        mapi_push_raw(S, registry_key);
        mapi_registry_set_key(S);

        mapi_table_set(S);

        table++;
    }
}

MORPHINE_AUX void maux_construct_call(
    morphine_state_t S,
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

    mapi_errorf(S, "Function %s wasn't found in table", name);

found:;
    mapi_push_native(S, table->name, table->value);
    mapi_push_raw(S, registry_key);
    mapi_registry_set_key(S);

    mapi_calli(S, argc);
}
