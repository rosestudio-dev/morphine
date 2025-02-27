//
// Created by whyiskra on 3/23/24.
//

#include "impl.h"
#include "mark.h"
#include "morphine/core/instance.h"
#include "morphine/object/reference.h"
#include "morphine/utils/overflow.h"

bool gcstageI_sweep(morphine_instance_t I, size_t debt) {
    size_t checked = 0;

    {
        struct object *current = I->G.pools.sweep;
        while (current != NULL && debt >= checked) {
            struct object *prev = current->prev;

            size_t size = size_obj(I, current);
            overflow_add(size, checked, SIZE_MAX) {
                checked = SIZE_MAX;
            } else {
                checked += size;
            }

            objectI_free(I, current);
            current = prev;
        }

        I->G.pools.sweep = current;
        if (current != NULL) {
            current->next = NULL;
        }
    }

    if (unlikely(I->G.pools.sweep == NULL)) {
        if (I->G.stats.allocated > I->G.settings.threshold) {
            I->G.stats.prev_allocated = I->G.stats.allocated;
        } else {
            I->G.stats.prev_allocated = I->G.settings.threshold;
        }

        return false;
    }

    if (I->G.stats.debt > checked) {
        I->G.stats.debt -= checked;
    } else {
        I->G.stats.debt = 0;
    }

    return true;
}