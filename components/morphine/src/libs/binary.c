//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

static void send(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "sio");
            mapi_push_arg(U, 1);
            mapi_binary_to(U);
            maux_nb_leave();
    maux_nb_end
}

static void receive(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "sio");
            mapi_binary_from(U);
            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("send", send),
    MAUX_CONSTRUCT_FUNCTION("receive", receive),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_binary(void) {
    return (morphine_library_t) {
        .name = "binary",
        .sharedkey = NULL,
        .init = library_init
    };
}
