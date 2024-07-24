//
// Created by why-iskra on 12.06.2024.
//

#include <morphine.h>
#include <limits.h>
#include "morphine/libs/builtin.h"

#define ML_INTEGER_BITS (sizeof(ml_integer) * CHAR_BIT)

static void not(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            ml_integer result = ~a;
            mapi_push_integer(U, result);
            maux_nb_return();
    maux_nb_end
}

static void and(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            ml_integer b = mapi_get_integer(U);

            ml_integer result = a & b;
            mapi_push_integer(U, result);
            maux_nb_return();
    maux_nb_end
}

static void or(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            ml_integer b = mapi_get_integer(U);

            ml_integer result = a | b;
            mapi_push_integer(U, result);
            maux_nb_return();
    maux_nb_end
}

static void xor(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            ml_integer b = mapi_get_integer(U);

            ml_integer result = a ^ b;
            mapi_push_integer(U, result);
            maux_nb_return();
    maux_nb_end
}

static void shl(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            ml_size b = mapi_get_size(U);

            ml_integer result = a << b;
            mapi_push_integer(U, result);
            maux_nb_return();
    maux_nb_end
}

static void shr(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            ml_size b = mapi_get_size(U);

            ml_integer result = a >> b;
            mapi_push_integer(U, result);
            maux_nb_return();
    maux_nb_end
}

static void rtl(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            ml_size b = mapi_get_size(U);

            ml_integer result = (a << (b % ML_INTEGER_BITS)) |
                                (a >> ((ML_INTEGER_BITS - b) % ML_INTEGER_BITS));
            mapi_push_integer(U, result);
            maux_nb_return();
    maux_nb_end
}

static void rtr(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            ml_size b = mapi_get_size(U);

            ml_integer result = (a >> (b % ML_INTEGER_BITS)) |
                                (a << ((ML_INTEGER_BITS - b) % ML_INTEGER_BITS));
            mapi_push_integer(U, result);
            maux_nb_return();
    maux_nb_end
}

static morphine_library_function_t functions[] = {
    { "not", not },
    { "and", and },
    { "or",  or },
    { "xor", xor },
    { "shl", shl },
    { "shr", shr },
    { "rtl", rtl },
    { "rtr", rtr },
    { NULL, NULL }
};

static morphine_library_t library = {
    .name = "bitwise",
    .types = NULL,
    .functions = functions,
    .integers = NULL,
    .decimals = NULL,
    .strings = NULL
};

MORPHINE_LIB morphine_library_t *mlib_builtin_bitwise(void) {
    return &library;
}
