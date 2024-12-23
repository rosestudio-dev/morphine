//
// Created by whyiskra on 3/23/24.
//

#include "impl.h"
#include "mark.h"
#include "morphine/utils/overflow.h"

static inline void record(morphine_instance_t I) {
    {
        morphine_coroutine_t current = I->interpreter.coroutines;
        while (current != NULL) {
            mark_object(I, objectI_cast(current));
            current = current->prev;
        }

        if (I->interpreter.running != NULL) {
            mark_object(I, objectI_cast(I->interpreter.running));
        }

        if (I->interpreter.next != NULL) {
            mark_object(I, objectI_cast(I->interpreter.next));
        }

        if (I->throw.context != NULL) {
            mark_object(I, objectI_cast(I->throw.context));
        }

        if (I->G.finalizer.coroutine != NULL) {
            mark_object(I, objectI_cast(I->G.finalizer.coroutine));
        }

        if (I->main != NULL) {
            mark_object(I, objectI_cast(I->main));
        }
    }

    {
        struct object *current = I->G.pools.black_coroutines;
        while (current != NULL) {
            mark_internal(I, current);
            current = current->prev;
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

    if (I->env != NULL) {
        mark_object(I, objectI_cast(I->env));
    }

    if (I->localstorage != NULL) {
        mark_object(I, objectI_cast(I->localstorage));
    }

    if (I->sharedstorage != NULL) {
        mark_object(I, objectI_cast(I->sharedstorage));
    }

    {
        for (size_t i = 0; i < I->G.safe.values.occupied; i++) {
            mark_value(I, I->G.safe.values.stack[i]);
        }
    }
}

bool gcstageI_increment(morphine_instance_t I, size_t debt) {
    record(I);

    size_t checked = 0;
    {
        struct object *current = I->G.pools.grey;
        while (current != NULL) {
            struct object *prev = current->prev;

            current->color = OBJ_COLOR_BLACK;
            gcI_pools_remove(current, &I->G.pools.grey);

            if (current->type == OBJ_TYPE_COROUTINE) {
                gcI_pools_insert(current, &I->G.pools.black_coroutines);
            } else {
                gcI_pools_insert(current, &I->G.pools.black);
            }

            size_t size = mark_internal(I, current);
            overflow_add(size, checked, SIZE_MAX) {
                checked = SIZE_MAX;
            } else {
                checked += size;
            }

            current = prev;

            if (unlikely(debt < checked)) {
                break;
            }
        }
    }

    if (I->G.stats.debt > checked) {
        I->G.stats.debt -= checked;
    } else {
        I->G.stats.debt = 0;
    }

    return I->G.pools.grey != NULL;
}
