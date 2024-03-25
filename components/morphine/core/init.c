//
// Created by whyiskra on 16.12.23.
//

#include "morphine/core/init.h"
#include "morphine/object/table.h"
#include "morphine/object/state.h"
#include "morphine/object/string.h"
#include "morphine/object/native.h"
#include "morphine/core/instance.h"
#include "morphine/api.h"
#include "morphine/auxiliary.h"

static void require(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t count = maux_checkargs_or(S, 1, 2);

            if (count == 1) {
                mapi_push_arg(S, 0);
                mapi_require(S);
                nb_return();
            } else {
                mapi_push_arg(S, 0);
                mapi_push_arg(S, 1);
                mapi_require_get(S);
                nb_return();
            }
    nb_end
}

static void init_metatable_names(morphine_instance_t I) {
    for (enum metatable_field field = MFS_START; field < MFS_COUNT; field++) {
        const char *name = metatableI_field2string(I, field);
        I->metatable.names[field] = stringI_create(I, name);
    }
}

static void init_metatable_defaults(morphine_instance_t I) {
    for (enum value_type type = VALUE_TYPES_START; type < VALUE_TYPES_COUNT; type++) {
        I->metatable.defaults[type] = NULL;
    }
}

static struct table *init_default_env(morphine_instance_t I) {
    struct table *env = tableI_create(I, 1);
    struct value key = valueI_object(stringI_create(I, "require"));
    struct value value = valueI_object(nativeI_create(I, "require", require));

    tableI_set(I, env, key, value);

    return env;
}

static void init_env(morphine_instance_t I) {
    I->env = init_default_env(I);
}

static void init_registry(morphine_instance_t I) {
    I->registry = tableI_create(I, 1);
}

void initI_vm(morphine_instance_t I) {
    init_metatable_names(I);
    init_metatable_defaults(I);
    init_env(I);
    init_registry(I);
}
