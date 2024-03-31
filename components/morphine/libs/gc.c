//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include <string.h>
#include "morphine/libs/loader.h"

static void full(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "empty");
            mapi_gc_full(mapi_instance(S));
            nb_leave();
    nb_end
}

static void force(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "empty");
            mapi_gc_force(mapi_instance(S));
            nb_leave();
    nb_end
}

static void step(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs(S, 2, "integer", "empty");

            size_t count = 1;
            if (variant == 0) {
                mapi_push_arg(S, 0);
                count = mapi_get_size(S);
            }

            for (size_t i = 0; i < count; i++) {
                mapi_gc_work(mapi_instance(S));
            }

            nb_leave();
    nb_end
}

static void isrunning(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "empty");
            bool result = mapi_gc_isrunning(mapi_instance(S));
            nb_return(mapi_push_boolean(S, result));
    nb_end
}

static void enable(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs(S, 2, "boolean", "empty");

            bool value = true;
            if (variant == 0) {
                mapi_push_arg(S, 0);
                value = mapi_get_boolean(S);
            }

            if (value) {
                mapi_gc_enable(mapi_instance(S));
            } else {
                mapi_gc_disable(mapi_instance(S));
            }
            nb_leave();
    nb_end
}

static void isenabled(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "empty");
            bool result = mapi_gc_isenabled(mapi_instance(S));
            nb_return(mapi_push_boolean(S, result));
    nb_end
}

static void changestart(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "integer");
            mapi_push_arg(S, 0);
            size_t value = mapi_get_size(S);
            mapi_gc_change_start(mapi_instance(S), value);
            nb_leave();
    nb_end
}

static void changedeal(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "integer");
            mapi_push_arg(S, 0);
            size_t value = mapi_get_size(S);
            mapi_gc_change_deal(mapi_instance(S), value);
            nb_leave();
    nb_end
}

static void changegrow(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "integer");
            mapi_push_arg(S, 0);
            size_t value = mapi_get_size(S);
            mapi_gc_change_grow(mapi_instance(S), value);
            nb_leave();
    nb_end
}

static void changefinalizerstacklimit(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "integer");
            mapi_push_arg(S, 0);
            size_t value = mapi_get_size(S);
            mapi_gc_change_finalizer_stack_limit(mapi_instance(S), value);
            nb_leave();
    nb_end
}

static void changefinalizerstackgrow(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "integer");
            mapi_push_arg(S, 0);
            size_t value = mapi_get_size(S);
            mapi_gc_change_finalizer_stack_grow(mapi_instance(S), value);
            nb_leave();
    nb_end
}

static void changestacklimit(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "integer");
            mapi_push_arg(S, 0);
            size_t value = mapi_get_size(S);
            mapi_gc_change_stack_limit(S, value);
            nb_leave();
    nb_end
}

static void changestackgrow(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "integer");
            mapi_push_arg(S, 0);
            size_t value = mapi_get_size(S);
            mapi_gc_change_stack_grow(S, value);
            nb_leave();
    nb_end
}

static void getmaxallocated(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "empty");
            size_t result = mapi_gc_max_allocated(mapi_instance(S));
            nb_return(mapi_push_size(S, result));
    nb_end
}

static void getallocated(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "empty");
            size_t result = mapi_gc_allocated(mapi_instance(S));
            nb_return(mapi_push_size(S, result));
    nb_end
}

static void getstacksize(morphine_state_t S) {
    nb_function(S)
        nb_init
            maux_checkargs(S, 1, "empty");
            size_t result = mapi_stack_size(S);
            nb_return(mapi_push_size(S, result));
    nb_end
}

static struct maux_construct_field table[] = {
    { "full",                      full },
    { "force",                     force },
    { "step",                      step },
    { "isrunning",                 isrunning },
    { "enable",                    enable },
    { "isenabled",                 isenabled },
    { "changestart",               changestart },
    { "changedeal",                changedeal },
    { "changegrow",                changegrow },
    { "changefinalizerstacklimit", changefinalizerstacklimit },
    { "changefinalizerstackgrow",  changefinalizerstackgrow },
    { "changestacklimit",          changestacklimit },
    { "changestackgrow",           changestackgrow },
    { "getmaxallocated",           getmaxallocated },
    { "getallocated",              getallocated },
    { "getstacksize",              getstacksize },
    { NULL, NULL }
};

void mlib_gc_loader(morphine_state_t S) {
    maux_construct(S, table);
}

MORPHINE_LIB void mlib_gc_call(morphine_state_t S, const char *name, size_t argc) {
    maux_construct_call(S, table, name, argc);
}
