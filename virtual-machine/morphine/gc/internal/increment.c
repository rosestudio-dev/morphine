//
// Created by why on 3/23/24.
//

#include "functions.h"
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

bool gcf_increment(morphine_instance_t I) {
    size_t deal = 0;
    size_t expected_deal = (I->G.bytes.allocated * I->G.settings.deal) / 100;

    gcf_internal_record(I);
retry:
    {
        struct object *pool = I->G.pools.gray;
        struct object *current = pool;
        while (current != NULL) {
            struct object *prev = current->prev;

            current->prev = I->G.pools.white;
            I->G.pools.white = current;

            deal += mark_internal(I, current);

            current = prev;
        }

        I->G.pools.gray = NULL;
    }

    if (!movegray(I)) {
        return true;
    }

    if (expected_deal > deal) {
        goto retry;
    }

    return false;
}