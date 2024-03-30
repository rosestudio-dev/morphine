//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/core/libloaders.h"

static void kill(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs_fixed(S, 0);
            mapi_push_self(S);
            mapi_push_stringf(S, "__state");
            mapi_table_getoe(S);
            morphine_state_t state = mapi_get_state(S);

            mapi_state_kill(state);
            nb_leave();
    nb_end
}

static void status(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs_fixed(S, 0);
            mapi_push_self(S);
            mapi_push_stringf(S, "__state");
            mapi_table_getoe(S);
            morphine_state_t state = mapi_get_state(S);

            mapi_push_stringf(S, mapi_state_status(state));
            nb_return();
    nb_end
}

static void priority(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs_fixed(S, 1);
            mapi_push_self(S);
            mapi_push_stringf(S, "__state");
            mapi_table_getoe(S);
            morphine_state_t state = mapi_get_state(S);

            mapi_push_arg(S, 0);
            morphine_integer_t priority = mapi_get_integer(S);
            mapi_state_priority(state, (priority_t) priority);
            nb_leave();
    nb_end
}

static void wait(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs_fixed(S, 0);
            mapi_push_self(S);
            mapi_push_stringf(S, "__state");
            mapi_table_getoe(S);
            morphine_state_t state = mapi_get_state(S);

            if (mapi_state_isalive(state)) {
                nb_continue(0);
            } else {
                nb_leave();
            }
    nb_end
}

static void suspend(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs_fixed(S, 0);
            mapi_push_self(S);
            mapi_push_stringf(S, "__state");
            mapi_table_getoe(S);
            morphine_state_t state = mapi_get_state(S);

            mapi_state_suspend(state);
            nb_leave();
    nb_end
}

static void resume(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs_fixed(S, 0);
            mapi_push_self(S);
            mapi_push_stringf(S, "__state");
            mapi_table_getoe(S);
            morphine_state_t state = mapi_get_state(S);

            mapi_state_resume(state);
            nb_leave();
    nb_end
}

static void create_instance(morphine_state_t S) {
    mapi_push_stringf(S, "kill");
    mapi_push_native(S, "coroutine.instance.kill", kill);
    mapi_table_set(S);

    mapi_push_stringf(S, "status");
    mapi_push_native(S, "coroutine.instance.status", status);
    mapi_table_set(S);

    mapi_push_stringf(S, "priority");
    mapi_push_native(S, "coroutine.instance.priority", priority);
    mapi_table_set(S);

    mapi_push_stringf(S, "wait");
    mapi_push_native(S, "coroutine.instance.wait", wait);
    mapi_table_set(S);

    mapi_push_stringf(S, "suspend");
    mapi_push_native(S, "coroutine.instance.suspend", suspend);
    mapi_table_set(S);

    mapi_push_stringf(S, "resume");
    mapi_push_native(S, "coroutine.instance.resume", resume);
    mapi_table_set(S);

    maux_table_lock(S);
}

static void start(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs_fixed(S, 0);
            mapi_push_table(S);

            mapi_push_stringf(S, "__state");
            morphine_state_t state = mapi_push_state(S);
            mapi_table_set(S);

            mapi_copy(S, state, 0);

            mapi_push_self(S);
            mapi_push_stringf(S, "__callable");
            mapi_table_getoe(S);
            mapi_move(S, state);
            mapi_pop(S, 1);

            for (size_t i = 0; i < mapi_args(S); i++) {
                mapi_push_arg(S, i);
                mapi_move(S, state);
            }

            mapi_callself(state, mapi_args(S));

            create_instance(S);

            mapi_attach(state);
            nb_return();
    nb_end
}

static void create(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs_pattern(S, 1, "callable");
            mapi_push_table(S);

            mapi_push_stringf(S, "__callable");
            mapi_push_arg(S, 0);
            mapi_table_set(S);

            mapi_push_stringf(S, "start");
            mapi_push_native(S, "coroutine.start", start);
            mapi_table_set(S);

            maux_table_lock(S);

            nb_return();
    nb_end
}

static void current(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs_fixed(S, 0);
            mapi_push_table(S);

            mapi_push_stringf(S, "__state");
            mapi_push_current_state(S);
            mapi_table_set(S);

            create_instance(S);

            nb_return();
    nb_end
}

static struct maux_construct_field table[] = {
    { "create",  create },
    { "current", current },
    { NULL, NULL }
};

void mlib_coroutine_loader(morphine_state_t S) {
    maux_construct(S, table);
}

MORPHINE_LIB void mlib_coroutine_call(morphine_state_t S, const char *name, size_t argc) {
    maux_construct_call(S, table, name, argc);
}
