//
// Created by whyiskra on 28.12.23.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

static void tostr(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            if (mapi_metatable_test(U, "_mf_to_string")) {
                if (mapi_is_callable(U)) {
                    mapi_callself(U, 0);
                } else {
                    maux_nb_return();
                }
            } else if (mapi_is_type(U, "vector")) {
                maux_library_get(U, "vector", "tostr");
                mapi_calli(U, 1);
            } else if (mapi_is_type(U, "table")) {
                maux_library_get(U, "table", "tostr");
                mapi_calli(U, 1);
            } else {
                mapi_to_string(U);
                maux_nb_return();
            }
        maux_nb_state(1)
            mapi_push_result(U);
            maux_nb_return();
    maux_nb_end
}

static void toint(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            if (mapi_args(U) == 2) {
                mapi_push_arg(U, 1);
                ml_size base = mapi_get_size(U);

                mapi_push_arg(U, 0);
                mapi_to_based_integer(U, base);
            } else {
                maux_expect_args(U, 1);
                mapi_push_arg(U, 0);
                mapi_to_integer(U);
            }
            maux_nb_return();
    maux_nb_end
}

static void tosize(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            if (mapi_args(U) == 2) {
                mapi_push_arg(U, 1);
                ml_size base = mapi_get_size(U);

                mapi_push_arg(U, 0);
                mapi_to_based_size(U, base);
            } else {
                maux_expect_args(U, 1);
                mapi_push_arg(U, 0);
                mapi_to_size(U);
            }
            maux_nb_return();
    maux_nb_end
}

static void toindex(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            if (mapi_args(U) == 2) {
                mapi_push_arg(U, 1);
                ml_size base = mapi_get_size(U);

                mapi_push_arg(U, 0);
                mapi_to_based_index(U, base);
            } else {
                maux_expect_args(U, 1);
                mapi_push_arg(U, 0);
                mapi_to_index(U);
            }
            maux_nb_return();
    maux_nb_end
}

static void todec(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_to_decimal(U);
            maux_nb_return();
    maux_nb_end
}

static void tobool(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_to_boolean(U);
            maux_nb_return();
    maux_nb_end
}

static morphine_library_function_t functions[] = {
    { "tostr",   tostr },
    { "toint",   toint },
    { "tosize",  tosize },
    { "toindex", toindex },
    { "todec",   todec },
    { "tobool",  tobool },
    { NULL, NULL }
};

static morphine_library_t library = {
    .name = "value",
    .types = NULL,
    .functions = functions,
    .integers = NULL,
    .decimals = NULL,
    .strings = NULL
};

MORPHINE_LIB morphine_library_t *mlib_builtin_value(void) {
    return &library;
}
