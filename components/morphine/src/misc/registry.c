//
// Created by whyiskra on 07.01.24.
//

#include "morphine/misc/registry.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"
#include "morphine/object/function.h"
#include "morphine/object/native.h"
#include "morphine/object/table.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/string.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"

void registryI_set_key(morphine_instance_t I, struct value callable, struct value key) {
    struct value source = callstackI_extract_callable(I, callable);

    if (valueI_is_function(source)) {
        struct function *function = valueI_as_function(source);
        function->registry_key = key;
        gcI_barrier(I, function, key);
    } else if (valueI_is_native(source)) {
        struct native *native = valueI_as_native(source);
        native->registry_key = key;
        gcI_barrier(I, native, key);
    }
}

void registryI_set(morphine_coroutine_t U, struct value key, struct value value) {
    morphine_instance_t I = U->I;
    struct value source = *callstackI_info_or_error(U)->s.source;

    struct value registry_key;
    if (valueI_is_function(source)) {
        registry_key = valueI_as_function(source)->registry_key;
    } else if (valueI_is_native(source)) {
        registry_key = valueI_as_native(source)->registry_key;
    } else {
        throwI_error(I, "attempt to set to registry for unsupported callable");
    }

    size_t rollback = gcI_safe_value(I, key);
    gcI_safe_value(I, value);

    bool has = false;
    struct value table = tableI_get(I, I->registry, registry_key, &has);

    if (!has) {
        table = valueI_object(tableI_create(I));
        gcI_safe_value(I, table);

        tableI_set(I, I->registry, registry_key, table);
    }

    tableI_set(I, valueI_as_table_or_error(I, table), key, value);

    gcI_reset_safe(I, rollback);
}

struct value registryI_get(morphine_coroutine_t U, struct value key, bool *has) {
    struct value source = *callstackI_info_or_error(U)->s.source;

    struct value registry_key;
    if (valueI_is_function(source)) {
        registry_key = valueI_as_function(source)->registry_key;
    } else if (valueI_is_native(source)) {
        registry_key = valueI_as_native(source)->registry_key;
    } else {
        throwI_error(U->I, "attempt to get from registry for unsupported callable");
    }

    struct value table = tableI_get(U->I, U->I->registry, registry_key, has);

    if (!(*has)) {
        return valueI_nil;
    }

    return tableI_get(U->I, valueI_as_table_or_error(U->I, table), key, has);
}

void registryI_clear(morphine_coroutine_t U) {
    struct value source = *callstackI_info_or_error(U)->s.source;

    struct value registry_key;
    if (valueI_is_function(source)) {
        registry_key = valueI_as_function(source)->registry_key;
    } else if (valueI_is_native(source)) {
        registry_key = valueI_as_native(source)->registry_key;
    } else {
        throwI_error(U->I, "attempt to clear registry for unsupported callable");
    }

    tableI_remove(U->I, U->I->registry, registry_key, NULL);
}

void registryI_clear_by_key(morphine_instance_t I, struct value key) {
    tableI_remove(I, I->registry, key, NULL);
}
