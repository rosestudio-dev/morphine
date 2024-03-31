//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include <string.h>
#include "morphine/libs/loader.h"

static void get(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "any");
            mapi_push_arg(S, 0);
            mapi_registry_get(S);
            nb_return();
    nb_end
}

static void has(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "any");
            mapi_push_arg(S, 0);
            bool has = mapi_registry_get(S);
            mapi_pop(S, 1);
            mapi_push_boolean(S, has);
            nb_return();
    nb_end
}

static void set(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "any,any");
            mapi_push_arg(S, 0);
            mapi_push_arg(S, 1);
            mapi_registry_set(S);
            nb_leave();
    nb_end
}

static void clear(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "empty");
            mapi_registry_clear(S);
            nb_leave();
    nb_end
}

static struct maux_construct_field table[] = {
    { "get",   get },
    { "set",   set },
    { "has",   has },
    { "clear", clear },
    { NULL, NULL }
};

void mlib_registry_loader(morphine_state_t S) {
    maux_construct(S, table);
}

MORPHINE_LIB void mlib_registry_call(morphine_state_t S, const char *name, size_t argc) {
    maux_construct_call(S, table, name, argc);
}
