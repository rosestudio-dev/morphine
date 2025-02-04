//
// Created by why-iskra on 01.09.2024.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

static void create(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            mapi_push_arg(U, 1);
            mapi_push_closure(U);
            maux_nb_return();
    maux_nb_end
}

static void lock(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_closure_lock(U);
            maux_nb_leave();
    maux_nb_end
}

static void unlock(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_closure_unlock(U);
        maux_nb_leave();
    maux_nb_end
}

static void value(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_closure_value(U);
            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("create", create),
    MAUX_CONSTRUCT_FUNCTION("lock", lock),
    MAUX_CONSTRUCT_FUNCTION("unlock", unlock),
    MAUX_CONSTRUCT_FUNCTION("value", value),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_closure(void) {
    return (morphine_library_t) {
        .name = "closure",
        .sharedkey = NULL,
        .init = library_init
    };
}
