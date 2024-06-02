//
// Created by whyiskra on 28.12.23.
//

#include <morphine.h>
#include "morphine/libs/loader.h"

static void tostr(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            if (mapi_metatable_test(U, "_mf_to_string")) {
                mapi_callself(U, 0);
            } else if(mapi_is_type(U, "vector")) {
                mlib_vector_call(U, "tostr", 1);
            } else if(mapi_is_type(U, "table")) {
                mlib_table_call(U, "tostr", 1);
            } else {
                mapi_to_string(U);
                nb_return();
            }
        nb_state(1)
            mapi_push_result(U);
            nb_return();
    nb_end
}

static void toint(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_to_integer(U);
            nb_return();
    nb_end
}

static void todec(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_to_decimal(U);
            nb_return();
    nb_end
}

static void tobool(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_to_boolean(U);
            nb_return();
    nb_end
}

static struct maux_construct_field table[] = {
    { "tostr",      tostr },
    { "toint",      toint },
    { "todec",      todec },
    { "tobool",     tobool },
    { NULL, NULL }
};

void mlib_value_loader(morphine_coroutine_t U) {
    maux_construct(U, table, "value.");
}

MORPHINE_LIB void mlib_value_call(morphine_coroutine_t U, const char *name, size_t argc) {
    maux_construct_call(U, table, "value.", name, argc);
}
