//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

static void lockmetatable(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "userdata");

            mapi_userdata_mode_lock_metatable(U);
            maux_nb_return();
    maux_nb_end
}

static void locksize(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "userdata");

            mapi_userdata_mode_lock_size(U);
            maux_nb_return();
    maux_nb_end
}

static void metatableislocked(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "userdata");

            bool value = mapi_userdata_mode_metatable_is_locked(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void sizeislocked(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "userdata");

            bool value = mapi_userdata_mode_size_is_locked(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static void isuntyped(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "userdata");

            bool value = mapi_userdata_is_untyped(U);
            mapi_push_boolean(U, value);
            maux_nb_return();
    maux_nb_end
}

static morphine_library_function_t functions[] = {
    { "isuntyped",         isuntyped },
    { "lockmetatable",     lockmetatable },
    { "locksize",          locksize },
    { "metatableislocked", metatableislocked },
    { "sizeislocked",      sizeislocked },
    { NULL, NULL }
};

static morphine_library_t library = {
    .name = "userdata",
    .types = NULL,
    .functions = functions,
    .integers = NULL,
    .decimals = NULL,
    .strings = NULL
};

MORPHINE_LIB morphine_library_t *mlib_builtin_userdata(void) {
    return &library;
}
