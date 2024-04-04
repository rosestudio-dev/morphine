//
// Created by whyiskra on 07.01.24.
//

#include "morphine/misc/registry.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"
#include "morphine/object/proto.h"
#include "morphine/object/native.h"
#include "morphine/object/table.h"
#include "morphine/object/coroutine.h"
#include "morphine/gc/barrier.h"
#include "morphine/stack/call.h"
#include "morphine/gc/safe.h"

void registryI_set_key(morphine_instance_t I, struct value callable, struct value key) {
    struct value source = callstackI_extract_callable(I, callable);

    if (valueI_is_proto(source)) {
        struct proto *proto = valueI_as_proto(source);
        proto->registry_key = key;
        gcI_barrier(proto, key);
    } else if (valueI_is_native(source)) {
        struct native *native = valueI_as_native(source);
        native->registry_key = key;
        gcI_barrier(native, key);
    }
}

void registryI_set(morphine_coroutine_t U, struct value key, struct value value) {
    morphine_instance_t I = U->I;
    struct value source = *callstackI_info_or_error(U)->s.source.p;

    struct value registry_key;
    if (valueI_is_proto(source)) {
        registry_key = valueI_as_proto(source)->registry_key;
    } else if (valueI_is_native(source)) {
        registry_key = valueI_as_native(source)->registry_key;
    } else {
        throwI_error(I, "Attempt to set to registry for unsupported callable");
    }

    gcI_safe(I, key);
    gcI_safe(I, value);

    bool has = false;
    struct value table = tableI_get(I, I->registry, registry_key, &has);

    if (!has) {
        table = valueI_object(tableI_create(I));
        gcI_safe(I, table);

        tableI_set(I, I->registry, registry_key, table);
    }

    tableI_set(I, valueI_as_table_or_error(I, table), key, value);

    gcI_reset_safe(I);
}

struct value registryI_get(morphine_coroutine_t U, struct value key, bool *has) {
    struct value source = *callstackI_info_or_error(U)->s.source.p;

    struct value registry_key;
    if (valueI_is_proto(source)) {
        registry_key = valueI_as_proto(source)->registry_key;
    } else if (valueI_is_native(source)) {
        registry_key = valueI_as_native(source)->registry_key;
    } else {
        throwI_error(U->I, "Attempt to get from registry for unsupported callable");
    }

    struct value table = tableI_get(U->I, U->I->registry, registry_key, has);

    if (!(*has)) {
        return valueI_nil;
    }

    return tableI_get(U->I, valueI_as_table_or_error(U->I, table), key, has);
}

void registryI_clear(morphine_coroutine_t U) {
    struct value source = *callstackI_info_or_error(U)->s.source.p;

    struct value registry_key;
    if (valueI_is_proto(source)) {
        registry_key = valueI_as_proto(source)->registry_key;
    } else if (valueI_is_native(source)) {
        registry_key = valueI_as_native(source)->registry_key;
    } else {
        throwI_error(U->I, "Attempt to clear registry for unsupported callable");
    }

    tableI_remove(U->I, U->I->registry, registry_key);
}
