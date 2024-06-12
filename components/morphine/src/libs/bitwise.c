//
// Created by why-iskra on 12.06.2024.
//

#include <morphine.h>
#include <limits.h>
#include "morphine/libs/loader.h"

#define ML_INTEGER_BITS (sizeof(ml_integer) * CHAR_BIT)

static void not(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            ml_integer result = ~a;
            mapi_push_integer(U, result);
            nb_return();
    nb_end
}

static void and(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            ml_integer b = mapi_get_integer(U);

            ml_integer result = a & b;
            mapi_push_integer(U, result);
            nb_return();
    nb_end
}

static void or(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            ml_integer b = mapi_get_integer(U);

            ml_integer result = a | b;
            mapi_push_integer(U, result);
            nb_return();
    nb_end
}

static void xor(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            ml_integer b = mapi_get_integer(U);

            ml_integer result = a ^ b;
            mapi_push_integer(U, result);
            nb_return();
    nb_end
}

static void shl(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            ml_size b = mapi_get_size(U);

            ml_integer result = a << b;
            mapi_push_integer(U, result);
            nb_return();
    nb_end
}

static void shr(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            ml_size b = mapi_get_size(U);

            ml_integer result = a >> b;
            mapi_push_integer(U, result);
            nb_return();
    nb_end
}

static void rtl(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            ml_size b = mapi_get_size(U);

            ml_integer result = (a << (b % ML_INTEGER_BITS)) |
                                (a >> ((ML_INTEGER_BITS - b) % ML_INTEGER_BITS));
            mapi_push_integer(U, result);
            nb_return();
    nb_end
}

static void rtr(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 0);
            ml_integer a = mapi_get_integer(U);

            mapi_push_arg(U, 1);
            ml_size b = mapi_get_size(U);

            ml_integer result = (a >> (b % ML_INTEGER_BITS)) |
                                (a << ((ML_INTEGER_BITS - b) % ML_INTEGER_BITS));
            mapi_push_integer(U, result);
            nb_return();
    nb_end
}

static struct maux_construct_field table[] = {
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

void mlib_bitwise_loader(morphine_coroutine_t U) {
    maux_construct(U, table, "bitwise.");
}

MORPHINE_LIB void mlib_bitwise_call(morphine_coroutine_t U, const char *name, ml_size argc) {
    maux_construct_call(U, table, "bitwise.", name, argc);
}
