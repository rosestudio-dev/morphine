//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include <string.h>
#include "morphine/libs/loader.h"

static void get(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_registry_get(U);
            maux_nb_return();
    maux_nb_end
}

static void has(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            bool has = mapi_registry_get(U);
            mapi_push_boolean(U, has);
            maux_nb_return();
    maux_nb_end
}

static void set(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            mapi_push_arg(U, 1);
            mapi_registry_set(U);
            maux_nb_leave();
    maux_nb_end
}

static void clear(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            mapi_registry_clear(U);
            maux_nb_leave();
    maux_nb_end
}

static struct maux_construct_field table[] = {
    { "get",   get },
    { "set",   set },
    { "has",   has },
    { "clear", clear },
    { NULL, NULL }
};

void mlib_registry_loader(morphine_coroutine_t U) {
    maux_construct(U, table, "registry.");
}

MORPHINE_LIB void mlib_registry_call(morphine_coroutine_t U, const char *name, ml_size argc) {
    maux_construct_call(U, table, "registry.", name, argc);
}
