//
// Created by whyiskra on 3/23/24.
//

#include "../stages.h"
#include "mark.h"
#include "morphine/core/instance.h"
#include "morphine/object/reference.h"

bool gcstageI_sweep(morphine_instance_t I, size_t debt) {
    size_t checked = 0;

    {
        struct object *current = I->G.pools.sweep;
        while (current != NULL && debt >= checked) {
            struct object *prev = current->prev;

            size_t size = size_obj(I, current);
            if (unlikely(size > SIZE_MAX - checked)) {
                checked = SIZE_MAX;
            } else {
                checked += size;
            }

            objectI_free(I, current);
            current = prev;
        }

        I->G.pools.sweep = current;
    }

    if (unlikely(I->G.pools.sweep == NULL)) {
        if (I->G.bytes.allocated > I->G.settings.threshold) {
            I->G.stats.prev_allocated = I->G.bytes.allocated;
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