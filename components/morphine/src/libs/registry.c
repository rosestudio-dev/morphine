//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include <string.h>
#include "morphine/libs/builtin.h"

static void get(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_registry_get(U);
            maux_nb_return();
    maux_nb_end
}

static void remove(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            bool has = mapi_registry_remove(U);
            mapi_push_boolean(U, has);
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

static morphine_library_function_t functions[] = {
    { "get",    get },
    { "set",    set },
    { "remove", remove },
    { "has",    has },
    { "clear",  clear },
    { NULL, NULL }
};

static morphine_library_t library = {
    .name = "registry",
    .types = NULL,
    .functions = functions,
    .integers = NULL,
    .decimals = NULL,
    .strings = NULL
};

MORPHINE_LIB morphine_library_t *mlib_builtin_registry(void) {
    return &library;
}
