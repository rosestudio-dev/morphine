//
// Created by whyiskra on 3/23/24.
//

#include "../stages.h"
#include "mark.h"
#include "morphine/core/instance.h"

bool gcstageI_finalize(morphine_instance_t I) {
    struct object *current = I->G.pools.allocated;
    struct object *pool = NULL;

    bool has = false;
    while (current != NULL) {
        struct object *prev = current->prev;

        if (unlikely(!current->flags.finalized && metatableI_test(I, valueI_object(current), MF_GC, NULL))) {
            current->prev = I->G.pools.finalize;
            I->G.pools.finalize = current;

            mark_internal(I, current);
            has = true;
        } else {
            current->prev = pool;
            pool = current;
        }

        current = prev;
    }

    I->G.pools.allocated = pool;

    if (has) {
        I->G.finalizer.work = true;
    }

    return has;
}