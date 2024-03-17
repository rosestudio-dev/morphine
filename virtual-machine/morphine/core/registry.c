//
// Created by whyiskra on 07.01.24.
//

#include "morphine/core/registry.h"
#include "morphine/core/stack.h"
#include "morphine/object/proto.h"
#include "morphine/object/native.h"
#include "morphine/object/table.h"
#include "morphine/object/state.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"


void registryI_set_key(morphine_state_t S, struct value callable, struct value key) {
    struct value source = stackI_extract_callable(S->I, callable);

    if (valueI_is_proto(source)) {
        struct proto *proto = valueI_as_proto(source);
        proto->registry_key = key;
        gcI_barrier(proto, key);
    } else if (valueI_is_native(source)) {
        struct native *native = valueI_as_native(source);
        native->registry_key = key;
        gcI_barrier(native, key);
    } else {
        throwI_message_error(S, "Attempt to change registry key for unsupported callable");
    }
}

void registryI_set(morphine_state_t S, struct value key, struct value value) {
    struct value source = *stackI_callinfo_or_error(S)->s.source.p;

    stackI_push(S, key);
    stackI_push(S, value);

    struct value registry_key;
    if (valueI_is_proto(source)) {
        registry_key = valueI_as_proto(source)->registry_key;
    } else if (valueI_is_native(source)) {
        registry_key = valueI_as_native(source)->registry_key;
    } else {
        throwI_message_error(S, "Attempt to set to registry for unsupported callable");
    }

    bool has = false;
    struct value table = tableI_get(S->I, S->I->registry, registry_key, &has);

    if (!has) {
        table = valueI_object(tableI_create(S->I, 1));
        stackI_push(S, table);

        tableI_set(S->I, S->I->registry, registry_key, table);

        stackI_pop(S, 1);
    }

    tableI_set(S->I, valueI_as_table_or_error(S, table), key, value);

    stackI_pop(S, 2);
}

struct value registryI_get(morphine_state_t S, struct value key, bool *has) {
    struct value source = *stackI_callinfo_or_error(S)->s.source.p;

    struct value registry_key;
    if (valueI_is_proto(source)) {
        registry_key = valueI_as_proto(source)->registry_key;
    } else if (valueI_is_native(source)) {
        registry_key = valueI_as_native(source)->registry_key;
    } else {
        throwI_message_error(S, "Attempt to get from registry for unsupported callable");
    }

    bool ihas = false;
    if (has == NULL) {
        has = &ihas;
    }

    struct value table = tableI_get(S->I, S->I->registry, registry_key, has);

    if (!(*has)) {
        return valueI_nil;
    }

    return tableI_get(S->I, valueI_as_table_or_error(S, table), key, has);
}

void registryI_clear(morphine_state_t S) {
    struct value source = *stackI_callinfo_or_error(S)->s.source.p;

    struct value registry_key;
    if (valueI_is_proto(source)) {
        registry_key = valueI_as_proto(source)->registry_key;
    } else if (valueI_is_native(source)) {
        registry_key = valueI_as_native(source)->registry_key;
    } else {
        throwI_message_error(S, "Attempt to clear registry for unsupported callable");
    }

    tableI_remove(S->I, S->I->registry, registry_key);
}
