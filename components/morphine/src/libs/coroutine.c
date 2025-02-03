//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/builtin.h"

#define GUARD_TYPE "coroutine.guard"

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
            maux_expect(U, MTYPE_COROUTINE);
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
            maux_expect(U, MTYPE_COROUTINE);
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_push_string(U, mapi_coroutine_status(coroutine));
            maux_nb_return();
    maux_nb_end
}

static void isalive(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_COROUTINE);
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_push_boolean(U, mapi_coroutine_is_alive(coroutine));
            maux_nb_return();
    maux_nb_end
}

static void name(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_COROUTINE);
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_coroutine_name(coroutine);
            maux_nb_return();
    maux_nb_end
}

static void stack_setlimit(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 2);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_COROUTINE);
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_push_arg(U, 1);
            ml_size value = mapi_get_size(U, NULL);

            mapi_stack_set_limit(coroutine, value);
            maux_nb_leave();
    maux_nb_end
}

static void wait(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_COROUTINE);
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
            maux_expect(U, MTYPE_COROUTINE);
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
            maux_expect(U, MTYPE_COROUTINE);
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_coroutine_resume(coroutine);
            maux_nb_leave();
    maux_nb_end
}

static void launch(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            if (mapi_args(U) < 2) {
                maux_expect_args(U, 2);
            }

            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_COROUTINE);
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_push_arg(U, 1);
            maux_expect(U, "callable");
            mapi_move(U, coroutine);

            for (ml_size i = 2; i < mapi_args(U); i++) {
                mapi_push_arg(U, i);
                mapi_move(U, coroutine);
            }

            mapi_call(coroutine, mapi_args(U) - 2);
            maux_nb_return();
    maux_nb_end
}

static void create(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            mapi_push_coroutine(U);
            maux_nb_return();
    maux_nb_end
}

static void guardlock(morphine_coroutine_t U) {
    maux_nb_function(U)
        maux_nb_init
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            mapi_peek(U, 0);
            maux_nb_operation("type", 1);
        maux_nb_state(1)
            mapi_push_result(U);
            if (mapi_string_cstr_compare(U, GUARD_TYPE) != 0) {
                mapi_error(U, "expected "GUARD_TYPE);
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
            maux_expect_args(U, 1);
            mapi_push_arg(U, 0);
            maux_expect(U, MTYPE_TABLE);

            mapi_peek(U, 0);
            maux_nb_operation("type", 1);
        maux_nb_state(1)
            mapi_push_result(U);
            if (mapi_string_cstr_compare(U, GUARD_TYPE) != 0) {
                mapi_error(U, "expected "GUARD_TYPE);
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

            maux_construct_element_t body[] = {
                MAUX_CONSTRUCT_BOOLEAN("islocked", false),
                MAUX_CONSTRUCT_FUNCTION("lock", guardlock),
                MAUX_CONSTRUCT_FUNCTION("unlock", guardunlock),
                MAUX_CONSTRUCT_END
            };

            maux_construct_element_t meta[] = {
                MAUX_CONSTRUCT_STRING(maux_metafield_name(U, MORPHINE_METAFIELD_TYPE), GUARD_TYPE),
                MAUX_CONSTRUCT_NIL(maux_metafield_name(U, MORPHINE_METAFIELD_MASK)),
                MAUX_CONSTRUCT_NIL(maux_metafield_name(U, MORPHINE_METAFIELD_SET)),
                MAUX_CONSTRUCT_END
            };

            maux_construct(U, body);
            maux_construct(U, meta);

            mapi_set_metatable(U);
            mapi_table_mode_fixed(U, true);
            mapi_table_mode_lock_metatable(U);
            mapi_table_mode_lock(U);

            maux_nb_return();
    maux_nb_end
}

static maux_construct_element_t elements[] = {
    MAUX_CONSTRUCT_FUNCTION("current", current),
    MAUX_CONSTRUCT_FUNCTION("create", create),
    MAUX_CONSTRUCT_FUNCTION("launch", launch),
    MAUX_CONSTRUCT_FUNCTION("resume", resume),
    MAUX_CONSTRUCT_FUNCTION("suspend", suspend),
    MAUX_CONSTRUCT_FUNCTION("kill", kill),
    MAUX_CONSTRUCT_FUNCTION("status", status),
    MAUX_CONSTRUCT_FUNCTION("isalive", isalive),
    MAUX_CONSTRUCT_FUNCTION("name", name),
    MAUX_CONSTRUCT_FUNCTION("stack.setlimit", stack_setlimit),
    MAUX_CONSTRUCT_FUNCTION("wait", wait),
    MAUX_CONSTRUCT_FUNCTION("guard", guard),
    MAUX_CONSTRUCT_END
};

static void library_init(morphine_coroutine_t U) {
    maux_construct(U, elements);
}

MORPHINE_LIB morphine_library_t mlib_builtin_coroutine(void) {
    return (morphine_library_t) {
        .name = "coroutine",
        .sharedkey = NULL,
        .init = library_init
    };
}
