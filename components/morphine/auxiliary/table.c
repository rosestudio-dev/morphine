//
// Created by whyiskra on 1/28/24.
//

#include "morphine/auxiliary/table.h"
#include "morphine/api.h"

static void setprotection(morphine_coroutine_t U) {
    mapi_errorf(U, "Table is protected from changes");
}

static void mask(morphine_coroutine_t U) {
    mapi_leave(U);
}

MORPHINE_AUX void maux_table_lock(morphine_coroutine_t U) {
    mapi_push_table(U);

    mapi_push_stringf(U, "_mf_mask");
    mapi_push_native(U, "aux.table.mask", mask);
    mapi_table_set(U);

    mapi_push_stringf(U, "_mf_set");
    mapi_push_native(U, "aux.table.setprotection", setprotection);
    mapi_table_set(U);

    mapi_set_metatable(U);
}
