//
// Created by whyiskra on 3/23/24.
//

#include "../stages.h"
#include "mark.h"
#include "morphine/core/instance.h"

size_t gcstageI_record(morphine_instance_t I) {
    size_t checked = 0;
    {
        morphine_coroutine_t current = I->E.coroutines;
        while (current != NULL) {
            mark_object(objectI_cast(current));
            checked += mark_internal(I, objectI_cast(current));
            current = current->prev;
        }

        if(I->E.running != NULL) {
            mark_object(objectI_cast(I->E.running));
            checked += mark_internal(I, objectI_cast(I->E.running));
        }

        if(I->E.next != NULL) {
            mark_object(objectI_cast(I->E.next));
            checked += mark_internal(I, objectI_cast(I->E.next));
        }

        if (I->G.finalizer.coroutine != NULL) {
            mark_object(objectI_cast(I->G.finalizer.coroutine));
            checked += mark_internal(I, objectI_cast(I->G.finalizer.coroutine));
        }
    }

    {
        struct object *current = I->G.pools.finalize;
        while (current != NULL) {
            checked += mark_internal(I, current);
            current = current->prev;
        }

        if (I->G.finalizer.candidate != NULL) {
            checked += mark_internal(I, I->G.finalizer.candidate);
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

    return checked;
}
