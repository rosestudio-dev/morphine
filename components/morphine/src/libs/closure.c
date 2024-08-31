//
// Created by why-iskra on 01.09.2024.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

static void create(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 1);
            ml_size size = mapi_get_size(U, NULL);
            mapi_push_arg(U, 0);
            mapi_push_closure(U, size);
            maux_nb_return();
    maux_nb_end
}

static void size(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            ml_size size = mapi_closure_size(U);
            mapi_push_size(U, size, NULL);
            maux_nb_return();
    maux_nb_end
}

static void get(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 1);
            ml_size size = mapi_get_size(U, NULL);
            mapi_push_arg(U, 0);
            mapi_closure_get(U, size);
            maux_nb_return();
    maux_nb_end
}

static void set(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 3);
            mapi_push_arg(U, 1);
            ml_size size = mapi_get_size(U, NULL);
            mapi_push_arg(U, 0);
            mapi_push_arg(U, 2);
            mapi_closure_set(U, size);
            maux_nb_leave();
    maux_nb_end
}

static morphine_library_function_t functions[] = {
    { "create", create },
    { "size",   size },
    { "get",    get },
    { "set",    set },
    { NULL, NULL }
};

static morphine_library_t library = {
    .name = "closure",
    .types = NULL,
    .functions = functions,
    .integers = NULL,
    .decimals = NULL,
    .strings = NULL
};

MORPHINE_LIB morphine_library_t *mlib_builtin_closure(void) {
    return &library;
}
