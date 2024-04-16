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

bool gcstageI_increment(morphine_instance_t I, bool full) {
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
    size_t record_checked = gcstageI_record(I);
    size_t expected = debt_calc(I);

retry:
    {
        struct object *current = I->G.pools.gray;
        while (current != NULL && (full || (expected >= checked))) {
            struct object *prev = current->prev;

            current->prev = I->G.pools.white;
            I->G.pools.white = current;

            checked += mark_internal(I, current);

            current = prev;
        }

        I->G.pools.gray = current;
    }

    bool has_gray = move_gray(I);

    if (has_gray && (full || (expected >= checked))) {
        goto retry;
    }

    if (I->G.stats.debt >= checked + record_checked) {
        I->G.stats.debt -= checked + record_checked;
    } else {
        I->G.stats.debt = 0;
    }

    return I->G.pools.gray != NULL;
}
