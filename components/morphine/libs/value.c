//
// Created by whyiskra on 28.12.23.
//

#include <morphine.h>
#include <stdio.h>
#include "morphine/libs/loader.h"

static void tostr(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs(S, 2, "self:any", "any");

            if (variant == 0) {
                mapi_push_self(S);
            } else {
                mapi_push_arg(S, 0);
            }

            if (mapi_metatable_test(S, "_mf_to_string")) {
                mapi_callself(S, 0);
            } else {
                mapi_to_string(S);
                nb_return();
            }
        nb_state(1)
            mapi_push_result(S);
            nb_return();
    nb_end
}

static void todec(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs(S, 2, "self:any", "any");

            if (variant == 0) {
                mapi_push_self(S);
            } else {
                mapi_push_arg(S, 0);
            }

            mapi_to_integer(S);
            nb_return();
    nb_end
}

static void tobaseddec(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs(S, 2, "self:string,integer", "string,integer");

            morphine_integer_t base;
            if (variant == 0) {
                mapi_push_arg(S, 0);
                base = mapi_get_integer(S);
                mapi_pop(S, 1);

                mapi_push_self(S);
            } else {
                mapi_push_arg(S, 1);
                base = mapi_get_integer(S);
                mapi_pop(S, 1);

                mapi_push_arg(S, 0);
            }

            mapi_to_based_integer(S, (uint8_t) base);
            nb_return();
    nb_end
}

static void todecimal(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs(S, 2, "self:any", "any");

            if (variant == 0) {
                mapi_push_self(S);
            } else {
                mapi_push_arg(S, 0);
            }

            mapi_to_decimal(S);
            nb_return();
    nb_end
}

static void tobool(morphine_state_t S) {
    nb_function(S)
        nb_init
            size_t variant = maux_checkargs(S, 2, "self:any", "any");

            if (variant == 0) {
                mapi_push_self(S);
            } else {
                mapi_push_arg(S, 0);
            }

            mapi_to_boolean(S);
            nb_return();
    nb_end
}

static struct maux_construct_field table[] = {
    { "tostr",      tostr },
    { "todec",      todec },
    { "tobaseddec", tobaseddec },
    { "todecimal",  todecimal },
    { "tobool",     tobool },
    { NULL, NULL }
};

void mlib_value_loader(morphine_state_t S) {
    maux_construct(S, table);
}

MORPHINE_LIB void mlib_value_call(morphine_state_t S, const char *name, size_t argc) {
    maux_construct_call(S, table, name, argc);
}
