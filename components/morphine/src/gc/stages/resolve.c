//
// Created by why-iskra on 16.04.2024.
//

#include "impl.h"
#include "mark.h"
#include "morphine/core/instance.h"
#include "morphine/object/reference.h"

static inline void invalidate_ref(struct reference *reference) {
    struct object *object = valueI_safe_as_object(reference->value, NULL);

    if (object == NULL) {
        return;
    }

    if (object->color != OBJ_COLOR_BLACK) {
        reference->value = valueI_nil;
    }
}

static inline void invalidate_coroutine(struct coroutine *coroutine) {
    stackI_shrink(coroutine);
}

static inline void invalidate(struct object *object) {
    if (object->type == OBJ_TYPE_REFERENCE) {
        invalidate_ref(cast(struct reference *, object));
    } else if (object->type == OBJ_TYPE_COROUTINE) {
        invalidate_coroutine(cast(struct coroutine *, object));
    }
}

static inline void invalidate_pool(struct object *pool) {
    struct object *current = pool;
    while (current != NULL) {
        invalidate(current);
        current = current->prev;
    }
}

static inline void resolve_pools(morphine_instance_t I) {
    invalidate_pool(I->G.pools.black);
    invalidate_pool(I->G.pools.finalize);

    if (I->G.finalizer.candidate != NULL) {
        invalidate(I->G.finalizer.candidate);
    }
}

static inline void resolve_cache(morphine_instance_t I, bool emergency) {
    struct callinfo *current = I->G.cache.callinfo.pool;
    while (current != NULL) {
        if (!emergency && I->G.cache.callinfo.size < I->G.settings.cache_callinfo_holding) {
            break;
        }

        struct callinfo *prev = current->prev;
        callstackI_callinfo_free(I, current);
        I->G.cache.callinfo.size--;

        current = prev;
    }

    I->G.cache.callinfo.pool = current;
}

static inline void attach_black_coroutines(morphine_instance_t I) {
    struct object *end = NULL;
    struct object *current = I->G.pools.black_coroutines;
    while (current != NULL) {
        end = current;
        current = current->prev;
    }

    if (end != NULL) {
        struct object *black = I->G.pools.black;

        if (black != NULL) {
            black->next = end;
        }
        end->prev = black;

        I->G.pools.black = I->G.pools.black_coroutines;
        I->G.pools.black_coroutines = NULL;
    }
}

static inline bool finalize(morphine_instance_t I) {
    struct object *current = I->G.pools.allocated;

    bool has = false;
    while (current != NULL) {
        struct object *prev = current->prev;

        if (unlikely(!current->flags.finalized && metatableI_test(I, valueI_object(current), MF_GC, NULL))) {
            current->color = OBJ_COLOR_RED;
            gcI_pools_remove(current, &I->G.pools.allocated);
            gcI_pools_insert(current, &I->G.pools.finalize);

            mark_internal(I, current);
            has = true;
        }

        current = prev;
    }

    if (has) {
        I->G.finalizer.work = true;
    }

    return has;
}

void gcstageI_resolve(morphine_instance_t I, bool emergency) {
    if (finalize(I)) {
        while (gcstageI_increment(I, SIZE_MAX)) { }
    }

    attach_black_coroutines(I);

    resolve_pools(I);
    resolve_cache(I, emergency);

    I->G.pools.sweep = I->G.pools.allocated;
    I->G.pools.allocated = I->G.pools.black;
    I->G.pools.black = NULL;

    I->G.stats.debt = 0;
    I->G.stats.prev_allocated = I->G.bytes.allocated;
}
