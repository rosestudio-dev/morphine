//
// Created by whyiskra on 28.12.23.
//

#include "morphine/libs/builtin.h"
#include <morphine.h>

static void toint(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
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

static void todec(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 1);
        mapi_push_arg(U, 0);
        mapi_to_decimal(U);
        maux_nb_return();
    maux_nb_end
}

static void tobool(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 1);
        mapi_push_arg(U, 0);
        mapi_to_boolean(U);
        maux_nb_return();
    maux_nb_end
}

static void tostr(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 1);
        mapi_push_arg(U, 0);
        maux_nb_operation("tostr", 1);

        maux_nb_state(1);
        mapi_push_result(U);
        maux_nb_return();
    maux_nb_end
}

static void compare(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 2);
        mapi_push_arg(U, 0);
        mapi_push_arg(U, 1);
        maux_nb_operation("compare", 1);

        maux_nb_state(1);
        mapi_push_result(U);
        maux_nb_return();
    maux_nb_end
}

static void hash(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init();
        maux_expect_args(U, 1);
        mapi_push_arg(U, 0);
        maux_nb_operation("hash", 1);

        maux_nb_state(1);
        mapi_push_result(U);
        maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("toint", toint),
    MAUX_CONSTRUCT_FUNCTION("todec", todec),
    MAUX_CONSTRUCT_FUNCTION("tobool", tobool),
    MAUX_CONSTRUCT_FUNCTION("tostr", tostr),
    MAUX_CONSTRUCT_FUNCTION("compare", compare),
    MAUX_CONSTRUCT_FUNCTION("hash", hash),
    MAUX_CONSTRUCT_INTEGER("limits.intmin", mm_typemin(ml_integer)),
    MAUX_CONSTRUCT_INTEGER("limits.intmax", mm_typemax(ml_integer)),
    MAUX_CONSTRUCT_INTEGER("limits.sizemax", mm_typemax(ml_size)),

#define def_type(s)                MAUX_CONSTRUCT_STRING("type."#s, #s),
#define mspec_type_object(i, n, s) def_type(s)
#define mspec_type_value(i, n, s)  def_type(s)

#include "morphine/core/type/specification.h"

#undef def_type
#undef mspec_type_object
#undef mspec_type_value

    MAUX_CONSTRUCT_END,
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_value(void) {
    return (morphine_library_t) {
        .name = "value",
        .sharedkey = NULL,
        .init = library_init,
    };
}
