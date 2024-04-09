//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/loader.h"

static void kill(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "self:table");
            mapi_push_self(U);
            mapi_push_stringf(U, "__coroutine");
            mapi_table_getoe(U);
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_coroutine_kill(coroutine);
            nb_leave();
    nb_end
}

static void status(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "self:table");
            mapi_push_self(U);
            mapi_push_stringf(U, "__coroutine");
            mapi_table_getoe(U);
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_push_stringf(U, mapi_coroutine_status(coroutine));
            nb_return();
    nb_end
}

static void priority(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "self:table,integer");
            mapi_push_self(U);
            mapi_push_stringf(U, "__coroutine");
            mapi_table_getoe(U);
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_push_arg(U, 0);
            ml_integer priority = mapi_get_integer(U);
            mapi_coroutine_priority(coroutine, (priority_t) priority);
            nb_leave();
    nb_end
}

static void wait(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "self:table");
            mapi_push_self(U);
            mapi_push_stringf(U, "__coroutine");
            mapi_table_getoe(U);
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            if (mapi_coroutine_is_alive(coroutine)) {
                nb_continue(0);
            } else {
                nb_leave();
            }
    nb_end
}

static void suspend(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "self:table");
            mapi_push_self(U);
            mapi_push_stringf(U, "__coroutine");
            mapi_table_getoe(U);
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_coroutine_suspend(coroutine);
            nb_leave();
    nb_end
}

static void resume(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "self:table");
            mapi_push_self(U);
            mapi_push_stringf(U, "__coroutine");
            mapi_table_getoe(U);
            morphine_coroutine_t coroutine = mapi_get_coroutine(U);

            mapi_coroutine_resume(coroutine);
            nb_leave();
    nb_end
}

static void create_instance(morphine_coroutine_t U) {
    mapi_push_stringf(U, "kill");
    mapi_push_native(U, "coroutine.instance.kill", kill);
    mapi_table_set(U);

    mapi_push_stringf(U, "status");
    mapi_push_native(U, "coroutine.instance.status", status);
    mapi_table_set(U);

    mapi_push_stringf(U, "priority");
    mapi_push_native(U, "coroutine.instance.priority", priority);
    mapi_table_set(U);

    mapi_push_stringf(U, "wait");
    mapi_push_native(U, "coroutine.instance.wait", wait);
    mapi_table_set(U);

    mapi_push_stringf(U, "suspend");
    mapi_push_native(U, "coroutine.instance.suspend", suspend);
    mapi_table_set(U);

    mapi_push_stringf(U, "resume");
    mapi_push_native(U, "coroutine.instance.resume", resume);
    mapi_table_set(U);

    maux_table_lock(U);
}

static void start(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "self:table");
            mapi_push_table(U);

            mapi_push_stringf(U, "__coroutine");
            morphine_coroutine_t coroutine = mapi_push_coroutine(U);
            mapi_table_set(U);

            mapi_copy(U, coroutine, 0);

            mapi_push_self(U);
            mapi_push_stringf(U, "__callable");
            mapi_table_getoe(U);
            mapi_move(U, coroutine);
            mapi_pop(U, 1);

            for (size_t i = 0; i < mapi_args(U); i++) {
                mapi_push_arg(U, i);
                mapi_move(U, coroutine);
            }

            mapi_callself(coroutine, mapi_args(U));

            create_instance(U);

            mapi_attach(coroutine);
            nb_return();
    nb_end
}

static void create(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "callable");
            mapi_push_table(U);

            mapi_push_stringf(U, "__callable");
            mapi_push_arg(U, 0);
            mapi_table_set(U);

            mapi_push_stringf(U, "start");
            mapi_push_native(U, "coroutine.start", start);
            mapi_table_set(U);

            maux_table_lock(U);

            nb_return();
    nb_end
}

static struct maux_construct_field table[] = {
    { "create",  create },
    { NULL, NULL }
};

void mlib_coroutine_loader(morphine_coroutine_t U) {
    maux_construct(U, table);
}

MORPHINE_LIB void mlib_coroutine_call(morphine_coroutine_t U, const char *name, size_t argc) {
    maux_construct_call(U, table, name, argc);
}
