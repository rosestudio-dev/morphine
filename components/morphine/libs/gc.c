//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include <string.h>
#include "morphine/libs/loader.h"

static void controller(morphine_state_t S) {
    nb_function(S)
        nb_init
            morphine_instance_t I = mapi_instance(S);
            maux_checkargs_minimum(S, 1);
            mapi_push_arg(S, 0);
            const char *name = mapi_get_string(S);

            if (strcmp(name, "full") == 0) {
                maux_checkargs_fixed(S, 1);
                mapi_gc_full(I);
            } else if (strcmp(name, "force") == 0) {
                maux_checkargs_fixed(S, 1);
                mapi_gc_force(I);
            } else if (strcmp(name, "step") == 0) {
                size_t size = maux_checkargs_or(S, 1, 2);
                morphine_integer_t count = 1;
                if (size == 2) {
                    mapi_push_arg(S, 1);
                    count = mapi_get_integer(S);
                    mapi_pop(S, 1);
                }

                for (morphine_integer_t i = 0; i < count; i++) {
                    mapi_gc_work(I);
                }
            } else if (strcmp(name, "isrunning") == 0) {
                maux_checkargs_fixed(S, 1);
                mapi_push_boolean(S, mapi_gc_isrunning(I));
                nb_return();
            } else if (strcmp(name, "enable") == 0) {
                maux_checkargs_fixed(S, 2);

                mapi_push_arg(S, 1);
                bool value = mapi_get_boolean(S);
                mapi_pop(S, 1);

                if (value) {
                    mapi_gc_enable(I);
                } else {
                    mapi_gc_disable(I);
                }
            } else if (strcmp(name, "isenabled") == 0) {
                maux_checkargs_fixed(S, 1);
                mapi_push_boolean(S, mapi_gc_isenabled(I));
                nb_return();
            } else if (strcmp(name, "changestart") == 0) {
                maux_checkargs_fixed(S, 2);

                mapi_push_arg(S, 1);
                morphine_integer_t value = mapi_get_integer(S);
                mapi_pop(S, 1);

                if (value < 0) {
                    mapi_errorf(S, "GC start value must be greater than zero");
                }

                mapi_gc_change_start(I, (size_t) value);
            } else if (strcmp(name, "changedeal") == 0) {
                maux_checkargs_fixed(S, 2);

                mapi_push_arg(S, 1);
                morphine_integer_t value = mapi_get_integer(S);
                mapi_pop(S, 1);

                if (value < 0) {
                    mapi_errorf(S, "GC deal value must be greater than zero");
                }

                mapi_gc_change_deal(I, (size_t) value);
            } else if (strcmp(name, "changegrow") == 0) {
                maux_checkargs_fixed(S, 2);

                mapi_push_arg(S, 1);
                morphine_integer_t value = mapi_get_integer(S);
                mapi_pop(S, 1);

                if (value < 0) {
                    mapi_errorf(S, "GC grow value must be greater than zero");
                }

                mapi_gc_change_grow(I, (size_t) value);
            } else if (strcmp(name, "changefinalizerstacklimit") == 0) {
                maux_checkargs_fixed(S, 2);

                mapi_push_arg(S, 1);
                morphine_integer_t value = mapi_get_integer(S);
                mapi_pop(S, 1);

                if (value < 0) {
                    mapi_errorf(S, "Stack limit value must be greater than zero");
                }

                mapi_gc_change_finalizer_stack_limit(I, (size_t) value);
            } else if (strcmp(name, "changefinalizerstackgrow") == 0) {
                maux_checkargs_fixed(S, 2);

                mapi_push_arg(S, 1);
                morphine_integer_t value = mapi_get_integer(S);
                mapi_pop(S, 1);

                if (value < 0) {
                    mapi_errorf(S, "Stack grow value must be greater than zero");
                }

                mapi_gc_change_finalizer_stack_grow(I, (size_t) value);
            } else if (strcmp(name, "changestacklimit") == 0) {
                maux_checkargs_fixed(S, 2);

                mapi_push_arg(S, 1);
                morphine_integer_t value = mapi_get_integer(S);
                mapi_pop(S, 1);

                if (value < 0) {
                    mapi_errorf(S, "Stack limit value must be greater than zero");
                }

                mapi_gc_change_stack_limit(S, (size_t) value);
            } else if (strcmp(name, "changestackgrow") == 0) {
                maux_checkargs_fixed(S, 2);

                mapi_push_arg(S, 1);
                morphine_integer_t value = mapi_get_integer(S);
                mapi_pop(S, 1);

                if (value < 0) {
                    mapi_errorf(S, "Stack grow value must be greater than zero");
                }

                mapi_gc_change_stack_grow(S, (size_t) value);
            } else if (strcmp(name, "getmaxallocated") == 0) {
                maux_checkargs_fixed(S, 1);
                mapi_push_integer(S, (morphine_integer_t) mapi_gc_max_allocated(I));
                nb_return();
            } else if (strcmp(name, "getallocated") == 0) {
                maux_checkargs_fixed(S, 1);
                mapi_push_integer(S, (morphine_integer_t) mapi_gc_allocated(I));
                nb_return();
            } else if (strcmp(name, "getstacksize") == 0) {
                maux_checkargs_fixed(S, 1);
                mapi_push_integer(S, (morphine_integer_t) mapi_stack_size(S));
                nb_return();
            } else {
                mapi_errorf(S, "Undefined '%s' gc command", name);
            }

            nb_leave();
    nb_end
}

static struct maux_construct_field table[] = {
    { "controller", controller },
    { NULL, NULL }
};

void mlib_gc_loader(morphine_state_t S) {
    maux_construct(S, table);
}

MORPHINE_LIB void mlib_gc_call(morphine_state_t S, const char *name, size_t argc) {
    maux_construct_call(S, table, name, argc);
}
