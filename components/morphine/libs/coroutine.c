//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/loader.h"

static void kill(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "coroutine");
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_coroutine_kill(coroutine);
            nb_leave();
    nb_end
}

static void status(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "coroutine");
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_push_stringf(U, mapi_coroutine_status(coroutine));
            nb_return();
    nb_end
}

static void priority(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "coroutine");
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "integer");
            ml_integer priority = mapi_get_integer(U);
            mapi_coroutine_priority(coroutine, (priority_t) priority);
            nb_leave();
    nb_end
}

static void wait(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "coroutine");
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            if (mapi_coroutine_is_alive(coroutine)) {
                mapi_pop(U, 2);
                nb_continue(0);
            } else {
                nb_leave();
            }
    nb_end
}

static void suspend(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "coroutine");
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_coroutine_suspend(coroutine);
            nb_leave();
    nb_end
}

static void resume(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "coroutine");
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_coroutine_resume(coroutine);
            nb_leave();
    nb_end
}

static void launch(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            mapi_push_arg(U, 0);
            maux_expect(U, "coroutine");
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_push_nil(coroutine);
            mapi_rotate(coroutine, 2);
            for (size_t i = 1; i < mapi_args(U); i++) {
                mapi_push_arg(U, i);
                mapi_move(U, coroutine);
            }

            mapi_callself(coroutine, mapi_args(U) - 1);

            mapi_attach(coroutine);
            nb_return();
    nb_end
}

static void create(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "callable");
            morphine_coroutine_t coroutine = mapi_push_coroutine(U);
            mapi_rotate(U, 2);
            mapi_copy(U, coroutine, 0);
            mapi_rotate(U, 2);

            nb_return();
    nb_end
}

static struct maux_construct_field table[] = {
    { "create",   create },
    { "launch",   launch },
    { "resume",   resume },
    { "suspend",  suspend },
    { "kill",     kill },
    { "status",   status },
    { "priority", priority },
    { "wait",     wait },
    { NULL, NULL }
};

void mlib_coroutine_loader(morphine_coroutine_t U) {
    maux_construct(U, table, "coroutine.");
}

MORPHINE_LIB void mlib_coroutine_call(morphine_coroutine_t U, const char *name, size_t argc) {
    maux_construct_call(U, table, "coroutine.", name, argc);
}
