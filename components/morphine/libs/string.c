//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/loader.h"

static struct maux_construct_field table[] = {
    { "format",       NULL },

    { "substring",    NULL },
    { "trim",         NULL },
    { "split",        NULL },
    { "replace",      NULL },
    { "replacefirst", NULL },
    { "replaceall",   NULL },
    { "lowercase",    NULL },
    { "uppercase",    NULL },
    { "repeat",       NULL },

    { "tochararray",  NULL },

    { "codeat",       NULL },
    { "charat",       NULL },
    { "indexof",      NULL },
    { "lastindexof",  NULL },

    { "startswith",   NULL },
    { "endswith",     NULL },
    { "isempty",      NULL },
    { "isblank",      NULL },
    { "contains",     NULL },
    { "count",       NULL },

    { NULL,           NULL },
};

void mlib_string_loader(morphine_state_t S) {
    maux_construct(S, table);
}

MORPHINE_LIB void mlib_string_call(morphine_state_t S, const char *name, size_t argc) {
    maux_construct_call(S, table, name, argc);
}
