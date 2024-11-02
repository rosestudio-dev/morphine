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

            if (mapi_metatable_builtin_test(U, MORPHINE_METAFIELD_TO_STRING)) {
                if (mapi_is_callable(U)) {
                    mapi_rotate(U, 2);
                    mapi_scall(U, 0);
                } else {
                    maux_nb_return();
                }
            } else if (mapi_is_type(U, "vector")) {
                maux_library_access(U, "vector.tostr");
                mapi_rotate(U, 2);
                mapi_call(U, 1);
            } else if (mapi_is_type(U, "table")) {
                maux_library_access(U, "table.tostr");
                mapi_rotate(U, 2);
                mapi_call(U, 1);
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

static void compare(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);

            if (mapi_metatable_builtin_test(U, MORPHINE_METAFIELD_COMPARE)) {
                if (mapi_is_callable(U)) {
                    mapi_rotate(U, 2);
                    mapi_push_arg(U, 1);
                    mapi_scall(U, 1);
                } else {
                    maux_nb_return();
                }
            } else {
                mapi_push_arg(U, 1);
                int result = mapi_compare(U);
                mapi_push_integer(U, result);
                maux_nb_return();
            }
        maux_nb_state(1)
            mapi_push_result(U);
            maux_nb_return();
    maux_nb_end
}

static void hash(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);

            if (mapi_metatable_builtin_test(U, MORPHINE_METAFIELD_HASH)) {
                if (mapi_is_callable(U)) {
                    mapi_rotate(U, 2);
                    mapi_scall(U, 0);
                } else {
                    maux_nb_return();
                }
            } else {
                ml_hash hash = mapi_hash(U);
                mapi_push_stringf(U, "%0*"MLIMIT_HASH_PR, sizeof(ml_hash) * 2, hash);
                maux_nb_return();
            }
        maux_nb_state(1)
            mapi_push_result(U);
            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("tostr", tostr),
    MAUX_CONSTRUCT_FUNCTION("toint", toint),
    MAUX_CONSTRUCT_FUNCTION("tosize", tosize),
    MAUX_CONSTRUCT_FUNCTION("todec", todec),
    MAUX_CONSTRUCT_FUNCTION("tobool", tobool),
    MAUX_CONSTRUCT_FUNCTION("compare", compare),
    MAUX_CONSTRUCT_FUNCTION("hash", hash),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_value(void) {
    return (morphine_library_t) {
        .name = "value",
        .sharedkey = NULL,
        .init = library_init
    };
}
