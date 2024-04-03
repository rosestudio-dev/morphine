//
// Created by whyiskra on 3/23/24.
//

#include "../stages.h"
#include "mark.h"
#include "morphine/core/instance.h"

static inline bool resolve_userdata(struct userdata *userdata) {
    struct link *current = userdata->links.pool;
    bool marked = false;
    while (current != NULL) {
        if (!current->soft) {
            marked = marked || mark_unmarked_object(objectI_cast(current->userdata));
        }

        current = current->prev;
    }

    return marked;
}

static inline bool movegray(morphine_instance_t I) {
    struct object *current = I->G.pools.allocated;
    struct object *pool = NULL;
    bool moved = false;
    while (current != NULL) {
        struct object *prev = current->prev;

        if (current->type == OBJ_TYPE_USERDATA) {
            moved = moved || resolve_userdata((struct userdata *) current);
        }

        if (current->flags.mark) {
            current->prev = I->G.pools.gray;
            I->G.pools.gray = current;
            moved = true;
        } else {
            current->prev = pool;
            pool = current;
        }

        current = prev;
    }

    I->G.pools.allocated = pool;

    return moved;
}

bool gcstageI_increment(morphine_instance_t I) {
    size_t checked = 0;
    gcstageI_record(I);
retry:
    {
        struct object *pool = I->G.pools.gray;
        struct object *current = pool;
        while (current != NULL) {
            struct object *prev = current->prev;

            current->prev = I->G.pools.white;
            I->G.pools.white = current;

            mark_internal(I, current);

            size_t temp = checked;
            checked++;
            if (unlikely(temp > checked)) {
                checked = SIZE_MAX;
            }

            current = prev;
        }

        I->G.pools.gray = NULL;
    }

    bool has_gray = movegray(I);

    if (has_gray) {
        size_t debtdiv = (I->G.stats.debt / 100);
        if (unlikely(debtdiv == 0)) {
            debtdiv = 1;
        }

        size_t percent = checked / debtdiv;

        if (percent < I->G.settings.deal) {
            goto retry;
        }
    }

    if (I->G.stats.debt >= checked) {
        I->G.stats.debt -= checked;
    }

    return !has_gray;
}