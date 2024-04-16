//
// Created by whyiskra on 3/23/24.
//

#include "../stages.h"
#include "mark.h"
#include "morphine/core/instance.h"
#include "morphine/object/reference.h"

static inline size_t debt_calc(morphine_instance_t I) {
    size_t conv = (uintmax_t) I->G.stats.debt;
    if (unlikely(conv == 0)) {
        conv = 1;
    }

    size_t percent = I->G.settings.deal / 10;
    if (unlikely(conv > SIZE_MAX / percent)) {
        return SIZE_MAX;
    }

    return (conv * percent) / 10;
}

static inline bool pause(morphine_instance_t I) {
    uint32_t size = ((uint32_t) 1) << I->G.settings.pause;
    if (unlikely(I->G.settings.pause == 0)) {
        size = 0;
    } else if (unlikely(I->G.settings.pause > 31)) {
        size = ((uint32_t) 1) << 31;
    }

    return I->G.stats.debt <= size;
}

bool gcstageI_sweep(morphine_instance_t I, bool full) {
    if (likely(!full)) {
        if (I->G.bytes.allocated > I->G.stats.prev_allocated) {
            size_t debt = I->G.bytes.allocated - I->G.stats.prev_allocated;

            if (likely(debt <= SIZE_MAX - I->G.stats.debt)) {
                I->G.stats.debt += debt;
            } else {
                I->G.stats.debt = SIZE_MAX;
            }
        }

        I->G.stats.prev_allocated = I->G.bytes.allocated;

        if (pause(I)) {
            return true;
        }
    }

    size_t checked = 0;
    size_t expected = debt_calc(I);

    {
        struct object *current = I->G.pools.sweep;
        while (current != NULL && (full || (expected >= checked))) {
            struct object *prev = current->prev;
            checked += size_obj(I, current);
            objectI_free(I, current);
            current = prev;
        }
        I->G.pools.sweep = current;
    }

    if (I->G.pools.sweep == NULL) {
        if (I->G.bytes.allocated > I->G.settings.threshold) {
            I->G.stats.prev_allocated = I->G.bytes.allocated;
        } else {
            I->G.stats.prev_allocated = I->G.settings.threshold;
        }

        return false;
    }

    if (I->G.stats.debt >= checked) {
        I->G.stats.debt -= checked;
    } else {
        I->G.stats.debt = 0;
    }

    return true;
}