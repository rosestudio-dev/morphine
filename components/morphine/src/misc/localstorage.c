//
// Created by whyiskra on 07.01.24.
//

#include "morphine/misc/localstorage.h"
#include "morphine/object/table.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/instance.h"
#include "morphine/gc/safe.h"

static inline struct value localkey(morphine_coroutine_t U) {
    return valueI_raw(callstackI_info(U));
}

void localstorageI_set(morphine_coroutine_t U, struct value key, struct value value) {
    morphine_instance_t I = U->I;
    struct value local_key = localkey(U);

    size_t rollback = gcI_safe_value(I, key);
    gcI_safe_value(I, value);

    bool has = false;
    struct value table = tableI_get(I, I->localstorage, local_key, &has);

    if (!has) {
        table = valueI_object(tableI_create(I));
        gcI_safe_value(I, table);

        tableI_set(I, I->localstorage, local_key, table);
    }

    tableI_set(I, valueI_as_table_or_error(I, table), key, value);

    gcI_reset_safe(I, rollback);
}

struct value localstorageI_get(morphine_coroutine_t U, struct value key, bool *has) {
    struct value local_key = localkey(U);

    bool internal_has = false;
    struct value table = tableI_get(U->I, U->I->localstorage, local_key, &internal_has);

    if (!internal_has) {
        if (has != NULL) {
            *has = internal_has;
        }

        return valueI_nil;
    }

    return tableI_get(U->I, valueI_as_table_or_error(U->I, table), key, has);
}

struct value localstorageI_remove(morphine_coroutine_t U, struct value key, bool *has) {
    struct value local_key = localkey(U);

    bool internal_has = false;
    struct value table = tableI_get(U->I, U->I->localstorage, local_key, &internal_has);

    if (!internal_has) {
        if (has != NULL) {
            *has = internal_has;
        }

        return valueI_nil;
    }

    return tableI_remove(U->I, valueI_as_table_or_error(U->I, table), key, has);
}

void localstorageI_clear(morphine_coroutine_t U) {
    struct value local_key = localkey(U);
    tableI_remove(U->I, U->I->localstorage, local_key, NULL);
}
