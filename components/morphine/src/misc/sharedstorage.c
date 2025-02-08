//
// Created by whyiskra on 07.01.24.
//

#include "morphine/misc/sharedstorage.h"
#include "morphine/core/instance.h"
#include "morphine/gc/safe.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/table.h"

bool sharedstorageI_define(morphine_instance_t I, const char *sharedkey) {
    gcI_safe_enter(I);
    struct string *string = gcI_safe_obj(I, string, stringI_create(I, sharedkey));

    bool has = false;
    tableI_get(I->sharedstorage, valueI_object(string), &has);

    if (!has) {
        struct table *table = gcI_safe_obj(I, table, tableI_create(I));
        tableI_set(I, I->sharedstorage, valueI_object(string), valueI_object(table));
    }

    gcI_safe_exit(I);

    return !has;
}

void sharedstorageI_set(morphine_instance_t I, const char *sharedkey, struct value key, struct value value) {
    gcI_safe_enter(I);
    gcI_safe(I, key);
    gcI_safe(I, value);

    struct string *string = gcI_safe_obj(I, string, stringI_create(I, sharedkey));

    bool has = false;
    struct value table = tableI_get(I->sharedstorage, valueI_object(string), &has);

    if (!has) {
        throwI_error(I, "undefined sharedstorage");
    }

    tableI_set(I, valueI_as_table_or_error(I, table), key, value);

    gcI_safe_exit(I);
}

struct value sharedstorageI_get(morphine_instance_t I, const char *sharedkey, struct value key, bool *has) {
    gcI_safe_enter(I);
    gcI_safe(I, key);

    struct string *string = gcI_safe_obj(I, string, stringI_create(I, sharedkey));

    bool internal_has = false;
    struct value table = tableI_get(I->sharedstorage, valueI_object(string), &internal_has);

    if (!internal_has) {
        throwI_error(I, "undefined sharedstorage");
    }

    struct value result = tableI_get(valueI_as_table_or_error(I, table), key, has);

    gcI_safe_exit(I);

    return result;
}

struct value sharedstorageI_remove(morphine_instance_t I, const char *sharedkey, struct value key, bool *has) {
    gcI_safe_enter(I);
    gcI_safe(I, key);

    struct string *string = gcI_safe_obj(I, string, stringI_create(I, sharedkey));

    bool internal_has = false;
    struct value table = tableI_get(I->sharedstorage, valueI_object(string), &internal_has);

    if (!internal_has) {
        throwI_error(I, "undefined sharedstorage");
    }

    struct value result = tableI_remove(I, valueI_as_table_or_error(I, table), key, has);

    gcI_safe_exit(I);

    return result;
}

void sharedstorageI_clear(morphine_instance_t I, const char *sharedkey) {
    gcI_safe_enter(I);
    struct string *string = gcI_safe_obj(I, string, stringI_create(I, sharedkey));

    bool internal_has = false;
    struct value table = tableI_get(I->sharedstorage, valueI_object(string), &internal_has);

    if (!internal_has) {
        throwI_error(I, "undefined sharedstorage");
    }

    tableI_clear(I, valueI_as_table_or_error(I, table));

    gcI_safe_exit(I);
}
