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
            ml_size b = mapi_get_size(U, "count");

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
            ml_size b = mapi_get_size(U, "count");

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
            ml_size b = mapi_get_size(U, "count");

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
            ml_size b = mapi_get_size(U, "count");

            ml_integer result = (a >> (b % ML_INTEGER_BITS)) |
                                (a << ((ML_INTEGER_BITS - b) % ML_INTEGER_BITS));
            mapi_push_integer(U, result);
            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("not", not),
    MAUX_CONSTRUCT_FUNCTION("and", and),
    MAUX_CONSTRUCT_FUNCTION("or",  or),
    MAUX_CONSTRUCT_FUNCTION("xor", xor),
    MAUX_CONSTRUCT_FUNCTION("shl", shl),
    MAUX_CONSTRUCT_FUNCTION("shr", shr),
    MAUX_CONSTRUCT_FUNCTION("rtl", rtl),
    MAUX_CONSTRUCT_FUNCTION("rtr", rtr),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_bitwise(void) {
    return (morphine_library_t) {
        .name = "bitwise",
        .sharedkey = NULL,
        .init = library_init
    };
}
