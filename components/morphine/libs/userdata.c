//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/loader.h"

static void ctype(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "userdata");

            mapi_push_string(U, mapi_userdata_type(U));
            nb_return();
    nb_end
}

static void lockmetatable(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "userdata");

            mapi_userdata_mode_lock_metatable(U);
            nb_return();
    nb_end
}

static void metatableislocked(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "userdata");

            bool value = mapi_userdata_mode_metatable_is_locked(U);
            mapi_push_boolean(U, value);
            nb_return();
    nb_end
}

static struct maux_construct_field table[] = {
    { "ctype",             ctype },
    { "lockmetatable",     lockmetatable },
    { "metatableislocked", metatableislocked },
    { NULL, NULL }
};

void mlib_userdata_loader(morphine_coroutine_t U) {
    maux_construct(U, table, "userdata.");
}

MORPHINE_LIB void mlib_userdata_call(morphine_coroutine_t U, const char *name, size_t argc) {
    maux_construct_call(U, table, "userdata.", name, argc);
}
