//
// Created by whyiskra on 3/23/24.
//

#include "impl.h"
#include "mark.h"
#include "morphine/core/instance.h"
#include "morphine/utils/overflow.h"

bool gcstageI_sweep(morphine_instance_t I, size_t debt) {
    size_t checked = 0;

    {
        struct object *current = I->G.pools.sweep;
        while (current != NULL && debt >= checked) {
            struct object *prev = current->prev;

            size_t size = size_obj(I, current);
            checked = mm_overflow_opd_add(size, checked, SIZE_MAX);

            objectI_free(I, current);
            current = prev;
        }

        I->G.pools.sweep = current;
        if (current != NULL) {
            current->next = NULL;
        }
    }

    if (mm_unlikely(I->G.pools.sweep == NULL)) {
        size_t allocated;
        if (I->G.stats.allocated > I->G.settings.threshold) {
            allocated = I->G.stats.allocated;
        } else {
            allocated = I->G.settings.threshold;
        }

        I->G.stats.prev_allocated = allocated;
        return false;
    }

    if (I->G.stats.debt > checked) {
        I->G.stats.debt -= checked;
    } else {
        I->G.stats.debt = 0;
    }

    return true;
}
