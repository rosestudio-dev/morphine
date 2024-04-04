//
// Created by whyiskra on 3/23/24.
//

#include "../stages.h"
#include "mark.h"
#include "morphine/core/instance.h"

void gcstageI_record(morphine_instance_t I) {
    {
        morphine_state_t current_state = I->E.states;
        while (current_state != NULL) {
            mark_object(objectI_cast(current_state));
            mark_internal(I, objectI_cast(current_state));
            current_state = current_state->prev;
        }

        morphine_state_t candidate = I->E.candidates;
        while (candidate != NULL) {
            mark_object(objectI_cast(candidate));
            mark_internal(I, objectI_cast(candidate));
            candidate = candidate->prev;
        }

        if (I->G.finalizer.state != NULL) {
            mark_object(objectI_cast(I->G.finalizer.state));
            mark_internal(I, objectI_cast(I->G.finalizer.state));
        }
    }

    {
        struct object *current = I->G.pools.finalize;
        while (current != NULL) {
            mark_internal(I, current);
            current = current->prev;
        }

        if (I->G.finalizer.candidate != NULL) {
            mark_internal(I, I->G.finalizer.candidate);
        }
    }

    for (enum metatable_field mf = MFS_START; mf < MFS_COUNT; mf++) {
        mark_object(objectI_cast(I->metatable.names[mf]));
    }

    for (enum value_type type = VALUE_TYPES_START; type < VALUE_TYPES_COUNT; type++) {
        struct table *table = I->metatable.defaults[type];
        if (table != NULL) {
            mark_object(objectI_cast(table));
        }
    }

    if (!I->E.throw.is_message) {
        mark_value(I->E.throw.error.value);
    }

    if (I->env != NULL) {
        mark_object(objectI_cast(I->env));
    }

    if (I->registry != NULL) {
        mark_object(objectI_cast(I->registry));
    }

    {
        size_t size = sizeof(I->G.safe.stack) / sizeof(struct value);

        for (size_t i = 0; i < size; i++) {
            mark_value(I->G.safe.stack[i]);
        }
    }
}
