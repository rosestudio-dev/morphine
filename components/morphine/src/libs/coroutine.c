//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

static void current(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            mapi_current(U);
            maux_nb_return();
    maux_nb_end
}

static void kill(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "coroutine");
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_coroutine_kill(coroutine);
            maux_nb_leave();
    maux_nb_end
}

static void status(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "coroutine");
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_push_string(U, mapi_coroutine_status(coroutine));
            maux_nb_return();
    maux_nb_end
}

static void name(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "coroutine");
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_coroutine_name(coroutine);
            maux_nb_return();
    maux_nb_end
}

static void priority(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, "coroutine");
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_push_arg(U, 1);
            ml_size priority = mapi_get_size(U, "priority");
            mapi_coroutine_priority(coroutine, priority);
            maux_nb_leave();
    maux_nb_end
}

static void wait(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "coroutine");
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            if (mapi_coroutine_is_alive(coroutine)) {
                mapi_stack_reset(U);
                maux_nb_continue(0);
            } else {
                maux_nb_leave();
            }
    maux_nb_end
}

static void suspend(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "coroutine");
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_coroutine_suspend(coroutine);
            maux_nb_leave();
    maux_nb_end
}

static void resume(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, "coroutine");
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_coroutine_resume(coroutine);
            maux_nb_leave();
    maux_nb_end
}

static void launch(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            if (mapi_args(U) < 1) {
                maux_expect_args(U, 1);
            }

            mapi_push_arg(U, 0);
            maux_expect(U, "coroutine");
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            for (ml_size i = 1; i < mapi_args(U); i++) {
                mapi_push_arg(U, i);
                mapi_move(U, coroutine);
            }

            mapi_scall(coroutine, mapi_args(U) - 1);

            mapi_attach(coroutine);
            maux_nb_return();
    maux_nb_end
}

static void create(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);

            mapi_push_arg(U, 1);
            maux_expect(U, "callable");

            mapi_push_arg(U, 0);
            morphine_coroutine_t coroutine = mapi_push_coroutine(U);

            mapi_rotate(U, 2);
            mapi_copy(U, coroutine, 0);
            mapi_pop(U, 1);

            mapi_push_self(U);
            mapi_move(U, coroutine);

            maux_nb_return();
    maux_nb_end
}

static void guardlock(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            mapi_push_self(U);
            maux_expect(U, "table");

            mapi_peek(U, 0);
        maux_nb_operation(1, "type")
            if (mapi_string_cstr_compare(U, "coroutine.guard") != 0) {
                mapi_error(U, "expected coroutine.guard");
            } else {
                mapi_pop(U, 1);
            }

            maux_nb_im_continue(2);
        maux_nb_state(2)
            mapi_push_string(U, "islocked");
            mapi_table_getoe(U);
            if (mapi_get_boolean(U)) {
                mapi_pop(U, 1);
                maux_nb_continue(2);
            } else {
                mapi_pop(U, 1);
                mapi_push_string(U, "islocked");
                mapi_push_boolean(U, true);
                mapi_table_set(U);
                maux_nb_leave();
            }
    maux_nb_end
}

static void guardunlock(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);
            mapi_push_self(U);
            maux_expect(U, "table");

            mapi_peek(U, 0);
        maux_nb_operation(1, "type")
            if (mapi_string_cstr_compare(U, "coroutine.guard") != 0) {
                mapi_error(U, "expected coroutine.guard");
            } else {
                mapi_pop(U, 1);
            }

            mapi_push_string(U, "islocked");
            mapi_push_boolean(U, false);
            mapi_table_set(U);
            maux_nb_leave();
    maux_nb_end
}

static void guard(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 0);

            mapi_push_table(U);

            mapi_push_string(U, "islocked");
            mapi_push_boolean(U, false);
            mapi_table_set(U);

            mapi_push_string(U, "lock");
            maux_push_native(U, "coroutine.guard.lock", guardlock);
            mapi_table_set(U);

            mapi_push_string(U, "unlock");
            maux_push_native(U, "coroutine.guard.unlock", guardunlock);
            mapi_table_set(U);

            mapi_push_table(U);

            mapi_push_string(U, "_mf_type");
            mapi_push_string(U, "coroutine.guard");
            mapi_table_set(U);

            mapi_push_string(U, "_mf_mask");
            mapi_push_nil(U);
            mapi_table_set(U);

            mapi_push_string(U, "_mf_set");
            mapi_push_nil(U);
            mapi_table_set(U);

            mapi_set_metatable(U);
            mapi_table_mode_fixed(U, true);
            mapi_table_mode_lock_metatable(U);
            mapi_table_mode_lock(U);

            maux_nb_return();
    maux_nb_end
}

static morphine_library_function_t functions[] = {
    { "current",  current },
    { "create",   create },
    { "launch",   launch },
    { "resume",   resume },
    { "suspend",  suspend },
    { "kill",     kill },
    { "status",   status },
    { "priority", priority },
    { "name",     name },
    { "wait",     wait },
    { "guard",    guard },
    { NULL, NULL }
};

static morphine_library_t library = {
    .name = "coroutine",
    .types = NULL,
    .functions = functions,
    .integers = NULL,
    .decimals = NULL,
    .strings = NULL
};

MORPHINE_LIB morphine_library_t *mlib_builtin_coroutine(void) {
    return &library;
}
