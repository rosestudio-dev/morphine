//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include <string.h>
#include "morphine/libs/loader.h"

static void full(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "empty");
            mapi_gc_full(mapi_instance(U));
            nb_leave();
    nb_end
}

static void force(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "empty");
            mapi_gc_force(mapi_instance(U));
            nb_leave();
    nb_end
}

static void step(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "integer", "empty");

            size_t count = 1;
            if (variant == 0) {
                mapi_push_arg(U, 0);
                count = mapi_get_size(U);
            }

            for (size_t i = 0; i < count; i++) {
                mapi_gc_work(mapi_instance(U));
            }

            nb_leave();
    nb_end
}

static void isrunning(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "empty");
            bool result = mapi_gc_is_running(mapi_instance(U));
            nb_return(mapi_push_boolean(U, result));
    nb_end
}

static void enable(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            size_t variant = maux_checkargs(U, 2, "boolean", "empty");

            bool value = true;
            if (variant == 0) {
                mapi_push_arg(U, 0);
                value = mapi_get_boolean(U);
            }

            if (value) {
                mapi_gc_enable(mapi_instance(U));
            } else {
                mapi_gc_disable(mapi_instance(U));
            }
            nb_leave();
    nb_end
}

static void isenabled(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "empty");
            bool result = mapi_gc_is_enabled(mapi_instance(U));
            nb_return(mapi_push_boolean(U, result));
    nb_end
}

static void changethreshold(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "integer");
            mapi_push_arg(U, 0);
            size_t value = mapi_get_size(U);
            mapi_gc_change_threshold(mapi_instance(U), value);
            nb_leave();
    nb_end
}

static void changedeal(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "integer");
            mapi_push_arg(U, 0);
            size_t value = mapi_get_size(U);
            mapi_gc_change_deal(mapi_instance(U), value);
            nb_leave();
    nb_end
}

static void changegrow(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "integer");
            mapi_push_arg(U, 0);
            size_t value = mapi_get_size(U);
            mapi_gc_change_grow(mapi_instance(U), value);
            nb_leave();
    nb_end
}

static void changefinalizerstacklimit(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "integer");
            mapi_push_arg(U, 0);
            size_t value = mapi_get_size(U);
            mapi_gc_change_finalizer_stack_limit(mapi_instance(U), value);
            nb_leave();
    nb_end
}

static void changefinalizerstackgrow(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "integer");
            mapi_push_arg(U, 0);
            size_t value = mapi_get_size(U);
            mapi_gc_change_finalizer_stack_grow(mapi_instance(U), value);
            nb_leave();
    nb_end
}

static void changestacklimit(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "integer");
            mapi_push_arg(U, 0);
            size_t value = mapi_get_size(U);
            mapi_gc_change_stack_limit(U, value);
            nb_leave();
    nb_end
}

static void changestackgrow(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "integer");
            mapi_push_arg(U, 0);
            size_t value = mapi_get_size(U);
            mapi_gc_change_stack_grow(U, value);
            nb_leave();
    nb_end
}

static void getmaxallocated(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "empty");
            size_t result = mapi_gc_max_allocated(mapi_instance(U));
            nb_return(mapi_push_size(U, result));
    nb_end
}

static void getallocated(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "empty");
            size_t result = mapi_gc_allocated(mapi_instance(U));
            nb_return(mapi_push_size(U, result));
    nb_end
}

static void status(morphine_coroutine_t U) {
    nb_function(U)
        nb_init
            maux_checkargs(U, 1, "empty");
            const char *result = mapi_gc_status(mapi_instance(U));
            nb_return(mapi_push_string(U, result));
    nb_end
}

static struct maux_construct_field table[] = {
    { "full",                      full },
    { "force",                     force },
    { "step",                      step },
    { "status",                    status },
    { "isrunning",                 isrunning },
    { "enable",                    enable },
    { "isenabled",                 isenabled },
    { "changethreshold",           changethreshold },
    { "changedeal",                changedeal },
    { "changegrow",                changegrow },
    { "changefinalizerstacklimit", changefinalizerstacklimit },
    { "changefinalizerstackgrow",  changefinalizerstackgrow },
    { "changestacklimit",          changestacklimit },
    { "changestackgrow",           changestackgrow },
    { "getmaxallocated",           getmaxallocated },
    { "getallocated",              getallocated },
    { NULL, NULL }
};

void mlib_gc_loader(morphine_coroutine_t U) {
    maux_construct(U, table);
}

MORPHINE_LIB void mlib_gc_call(morphine_coroutine_t U, const char *name, size_t argc) {
    maux_construct_call(U, table, name, argc);
}
