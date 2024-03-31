//
// Created by whyiskra on 30.12.23.
//

#include <morphine.h>
#include "morphine/libs/loader.h"

static struct maux_construct_field table[] = {
    { NULL, NULL }
};

void mlib_table_loader(morphine_state_t S) {
    maux_construct(S, table);
}

MORPHINE_LIB void mlib_table_call(morphine_state_t S, const char *name, size_t argc) {
    maux_construct_call(S, table, name, argc);
}
