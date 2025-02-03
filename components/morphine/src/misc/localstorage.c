//
// Created by whyiskra on 07.01.24.
//

#include "morphine/misc/localstorage.h"
#include "morphine/core/instance.h"
#include "morphine/gc/safe.h"
#include "morphine/object/table.h"

static inline struct value localstorage_key(struct callframe *frame) {
    return valueI_raw(frame);
}

static inline struct value localstorage_current_key(morphine_coroutine_t U) {
    callstackI_check_access(U);
    return localstorage_key(U->callstack.frame);
}

void localstorageI_set(morphine_coroutine_t U, struct value key, struct value value) {
    morphine_instance_t I = U->I;
    struct value local_key = localstorage_current_key(U);

    gcI_safe_enter(I);
    gcI_safe(I, key);
    gcI_safe(I, value);

    bool has = false;
    struct value table = tableI_get(I, I->localstorage, local_key, &has);

    if (!has) {
        table = valueI_object(tableI_create(I));
        gcI_safe(I, table);

        tableI_set(I, I->localstorage, local_key, table);
    }

    tableI_set(I, valueI_as_table_or_error(I, table), key, value);

    gcI_safe_exit(I);
}

struct value localstorageI_get(morphine_coroutine_t U, struct value key, bool *has) {
    struct value local_key = localstorage_current_key(U);

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
    struct value local_key = localstorage_current_key(U);

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

void localstorageI_clear(morphine_instance_t I, struct callframe *frame) {
    struct value local_key = localstorage_key(frame);
    tableI_remove(I, I->localstorage, local_key, NULL);
}
