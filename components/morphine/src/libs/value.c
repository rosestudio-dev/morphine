//
// Created by whyiskra on 28.12.23.
//

#include <morphine.h>
#include "morphine/libs/loader.h"

static void tostr(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            if (mapi_metatable_test(U, "_mf_to_string")) {
                mapi_callself(U, 0);
            } else if (mapi_is_type(U, "vector")) {
                mlib_vector_call(U, "tostr", 1);
            } else if (mapi_is_type(U, "table")) {
                mlib_table_call(U, "tostr", 1);
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

static struct maux_construct_field table[] = {
    { "tostr",   tostr },
    { "toint",   toint },
    { "tosize",  tosize },
    { "toindex", toindex },
    { "todec",   todec },
    { "tobool",  tobool },
    { NULL, NULL }
};

void mlib_value_loader(morphine_coroutine_t U) {
    maux_construct(U, table, "value.");
}

MORPHINE_LIB void mlib_value_call(morphine_coroutine_t U, const char *name, ml_size argc) {
    maux_construct_call(U, table, "value.", name, argc);
}
