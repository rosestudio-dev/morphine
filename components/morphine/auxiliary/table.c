//
// Created by whyiskra on 1/28/24.
//

#include "morphine/auxiliary/table.h"
#include "morphine/api.h"

static void setprotection(morphine_state_t S) {
    mapi_errorf(S, "Table is protected from changes");
}

static void mask(morphine_state_t S) {
    mapi_leave(S);
}

MORPHINE_AUX void maux_table_lock(morphine_state_t S) {
    mapi_push_table(S);

    mapi_push_stringf(S, "_mf_mask");
    mapi_push_native(S, "aux.table.mask", mask);
    mapi_table_set(S);

    mapi_push_stringf(S, "_mf_set");
    mapi_push_native(S, "aux.table.setprotection", setprotection);
    mapi_table_set(S);

    mapi_set_metatable(S);
}
