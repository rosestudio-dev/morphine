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
                mapi_library(U, "vector.tostr", false);
                mapi_calli(U, 1);
            } else if (mapi_is_type(U, "table")) {
                mapi_library(U, "table.tostr", false);
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
                ml_size base = mapi_get_size(U, "base");

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

                ml_size base = 10;
                mapi_push_string(U, "base");
                if (mapi_table_get(U)) {
                    base = mapi_get_size(U, "base");
                }
                mapi_pop(U, 1);

                const char *name = NULL;
                mapi_push_string(U, "name");
                if (mapi_table_get(U)) {
                    name = mapi_get_cstr(U);
                }
                mapi_pop(U, 1);

                mapi_push_arg(U, 0);
                if (base == 10) {
                    mapi_to_size(U, name);
                } else {
                    mapi_to_based_size(U, base, name);
                }
            } else {
                maux_expect_args(U, 1);
                mapi_push_arg(U, 0);
                mapi_to_size(U, NULL);
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
    { "tostr",  tostr },
    { "toint",  toint },
    { "tosize", tosize },
    { "todec",  todec },
    { "tobool", tobool },
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
