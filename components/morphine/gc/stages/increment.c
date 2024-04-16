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

static inline bool move_gray(morphine_instance_t I) {
    struct object *current = I->G.pools.allocated;
    struct object *pool = NULL;
    bool moved = false;
    while (current != NULL) {
        struct object *prev = current->prev;

        if (unlikely(current->type == OBJ_TYPE_USERDATA)) {
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

bool gcstageI_increment(morphine_instance_t I, bool full, size_t debt) {
    size_t checked = 0;
    size_t record_checked = gcstageI_record(I);

retry:
    {
        struct object *current = I->G.pools.gray;
        while (current != NULL && (full || (debt >= checked))) {
            struct object *prev = current->prev;

            current->prev = I->G.pools.white;
            I->G.pools.white = current;

            size_t size = mark_internal(I, current);
            if (unlikely(size > SIZE_MAX - checked)) {
                checked = SIZE_MAX;
            } else {
                checked += size;
            }

            current = prev;
        }

        I->G.pools.gray = current;
    }

    bool has_gray = move_gray(I);

    if (has_gray && (full || (debt >= checked))) {
        goto retry;
    }

    if (I->G.stats.debt >= checked + record_checked) {
        I->G.stats.debt -= checked + record_checked;
    } else {
        I->G.stats.debt = 0;
    }

    return I->G.pools.gray != NULL;
}
