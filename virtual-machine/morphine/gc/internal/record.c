//
// Created by why on 3/23/24.
//

#include "functions.h"
#include "morphine/core/instance.h"

void gcf_internal_record(morphine_instance_t I) {
    {
        morphine_state_t current_state = I->states;
        while (current_state != NULL) {
            mark_object(objectI_cast(current_state));
            mark_internal(I, objectI_cast(current_state));
            current_state = current_state->prev;
        }

        if (I->G.finalizer.state != NULL) {
            mark_object(objectI_cast(I->G.finalizer.state));
            mark_internal(I, objectI_cast(I->G.finalizer.state));
        }
    }

    {
        morphine_state_t current_state = I->candidates;
        while (current_state != NULL) {
            mark_object(objectI_cast(current_state));
            mark_internal(I, objectI_cast(current_state));
            current_state = current_state->prev;
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

    if (!I->throw.is_message) {
        mark_value(I->throw.result.value);
    }

    if (I->env != NULL) {
        mark_object(objectI_cast(I->env));
    }

    if (I->registry != NULL) {
        mark_object(objectI_cast(I->registry));
    }
}