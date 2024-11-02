//
// Created by whyiskra on 07.01.24.
//

#include "morphine/misc/sharedstorage.h"
#include "morphine/object/table.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/instance.h"
#include "morphine/gc/safe.h"

bool sharedstorageI_define(morphine_instance_t I, const char *sharedkey) {
    struct string *string = stringI_create(I, sharedkey);
    size_t rollback = gcI_safe_obj(I, objectI_cast(string));

    bool has = false;
    tableI_get(I, I->sharedstorage, valueI_object(string), &has);

    if (!has) {
        struct table *table = tableI_create(I);
        gcI_safe_obj(I, objectI_cast(table));

        tableI_set(I, I->sharedstorage, valueI_object(string), valueI_object(table));
    }

    gcI_reset_safe(I, rollback);

    return !has;
}

void sharedstorageI_set(
    morphine_instance_t I,
    const char *sharedkey,
    struct value key,
    struct value value
) {
    size_t rollback = gcI_safe_value(I, key);
    gcI_safe_value(I, value);

    struct string *string = stringI_create(I, sharedkey);
    gcI_safe_obj(I, objectI_cast(string));

    bool has = false;
    struct value table = tableI_get(I, I->sharedstorage, valueI_object(string), &has);

    if (!has) {
        throwI_error(I, "undefined sharedstorage");
    }

    tableI_set(I, valueI_as_table_or_error(I, table), key, value);

    gcI_reset_safe(I, rollback);
}

struct value sharedstorageI_get(
    morphine_instance_t I,
    const char *sharedkey,
    struct value key,
    bool *has
) {
    size_t rollback = gcI_safe_value(I, key);

    struct string *string = stringI_create(I, sharedkey);
    gcI_safe_obj(I, objectI_cast(string));

    bool internal_has = false;
    struct value table = tableI_get(I, I->sharedstorage, valueI_object(string), &internal_has);

    if (!internal_has) {
        throwI_error(I, "undefined sharedstorage");
    }

    struct value result = tableI_get(I, valueI_as_table_or_error(I, table), key, has);

    gcI_reset_safe(I, rollback);

    return result;
}

struct value sharedstorageI_remove(
    morphine_instance_t I,
    const char *sharedkey,
    struct value key,
    bool *has
) {
    size_t rollback = gcI_safe_value(I, key);

    struct string *string = stringI_create(I, sharedkey);
    gcI_safe_obj(I, objectI_cast(string));

    bool internal_has = false;
    struct value table = tableI_get(I, I->sharedstorage, valueI_object(string), &internal_has);

    if (!internal_has) {
        throwI_error(I, "undefined sharedstorage");
    }

    struct value result = tableI_remove(I, valueI_as_table_or_error(I, table), key, has);

    gcI_reset_safe(I, rollback);

    return result;
}

void sharedstorageI_clear(morphine_instance_t I, const char *sharedkey) {
    struct string *string = stringI_create(I, sharedkey);
    size_t rollback = gcI_safe_obj(I, objectI_cast(string));

    bool internal_has = false;
    struct value table = tableI_get(I, I->sharedstorage, valueI_object(string), &internal_has);

    if (!internal_has) {
        throwI_error(I, "undefined sharedstorage");
    }

    tableI_clear(I, valueI_as_table_or_error(I, table));

    gcI_reset_safe(I, rollback);
}
