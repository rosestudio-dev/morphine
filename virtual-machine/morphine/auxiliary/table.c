//
// Created by whyiskra on 1/28/24.
//

#include "morphine/auxiliary.h"
#include "morphine/api.h"

static void setprotection(morphine_state_t S) {
    mapi_errorf(S, "Table is protected from changes");
}

MORPHINE_AUX void maux_table_lock(morphine_state_t S) {
    mapi_push_table(S, 2);

    mapi_push_stringf(S, "_mf_mask");
    maux_push_empty_callable(S);
    mapi_table_set(S);

    mapi_push_stringf(S, "_mf_set");
    mapi_push_native(S, "aux.setprotection", setprotection);
    mapi_table_set(S);

    mapi_set_metatable(S);
}
