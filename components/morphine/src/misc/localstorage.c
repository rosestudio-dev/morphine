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

static inline struct value localstorage_current_key(morphine_instance_t I) {
    return localstorage_key(callstackI_interpreter_context(I));
}

void localstorageI_set(morphine_instance_t I, struct value key, struct value value) {
    struct value local_key = localstorage_current_key(I);

    gcI_safe_enter(I);
    gcI_safe(I, key);
    gcI_safe(I, value);

    bool has = false;
    struct value table = tableI_get(I->localstorage, local_key, &has);

    if (!has) {
        table = valueI_object(tableI_create(I));
        gcI_safe(I, table);

        tableI_set(I, I->localstorage, local_key, table);
    }

    tableI_set(I, valueI_as_table_or_error(I, table), key, value);

    gcI_safe_exit(I);
}

struct value localstorageI_get(morphine_instance_t I, struct value key, bool *has) {
    struct value local_key = localstorage_current_key(I);

    bool internal_has = false;
    struct value table = tableI_get(I->localstorage, local_key, &internal_has);

    if (!internal_has) {
        if (has != NULL) {
            *has = internal_has;
        }

        return valueI_nil;
    }

    return tableI_get(valueI_as_table_or_error(I, table), key, has);
}

struct value localstorageI_remove(morphine_instance_t I, struct value key, bool *has) {
    struct value local_key = localstorage_current_key(I);

    bool internal_has = false;
    struct value table = tableI_get(I->localstorage, local_key, &internal_has);

    if (!internal_has) {
        if (has != NULL) {
            *has = internal_has;
        }

        return valueI_nil;
    }

    return tableI_remove(I, valueI_as_table_or_error(I, table), key, has);
}

void localstorageI_clear(morphine_instance_t I) {
    struct value local_key = localstorage_current_key(I);
    tableI_remove(I, I->localstorage, local_key, NULL);
}

void localstorageI_clear_frame(morphine_instance_t I, struct callframe *frame) {
    struct value local_key = localstorage_key(frame);
    tableI_remove(I, I->localstorage, local_key, NULL);
}
