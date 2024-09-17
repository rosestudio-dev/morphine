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
    struct object *current = I->G.pools.black_coroutines;
    while (current != NULL) {
        gcI_pools_remove(current, &I->G.pools.black_coroutines);
        gcI_pools_insert(current, &I->G.pools.black);

        current = I->G.pools.black_coroutines;
    }
}

static inline bool simple_finalize_sio(morphine_instance_t I, struct sio *sio) {
    if (valueI_safe_as_sio(sio->hold_value, NULL) == sio) {
        return false;
    }

    return mark_value(I, sio->hold_value);
}

static inline bool simple_finalize(morphine_instance_t I, struct object *object) {
    if (object->type == OBJ_TYPE_SIO) {
        return simple_finalize_sio(I, cast(struct sio *, object));
    }

    return false;
}

static inline bool finalize(morphine_instance_t I) {
    struct object *current = I->G.pools.allocated;

    bool has_simple_finalized = false;
    while (current != NULL) {
        if (simple_finalize(I, current)) {
            has_simple_finalized = true;
        }

        current = current->prev;
    }

    current = I->G.pools.allocated;
    bool has_to_be_finalize = false;
    while (current != NULL) {
        struct object *prev = current->prev;

        if (unlikely(!current->flags.finalized && metatableI_test(I, valueI_object(current), MF_GC, NULL))) {
            current->color = OBJ_COLOR_RED;
            gcI_pools_remove(current, &I->G.pools.allocated);
            gcI_pools_insert(current, &I->G.pools.finalize);

            has_to_be_finalize = true;
        }

        current = prev;
    }

    if (has_to_be_finalize) {
        I->G.finalizer.work = true;
    }

    return has_to_be_finalize || has_simple_finalized;
}

static inline bool mark_sso(morphine_instance_t I) {
    bool marked = false;

#ifdef MORPHINE_ENABLE_SSO
    for (size_t r = 0; r < MPARAM_SSO_HASHTABLE_ROWS; r++) {
        for (size_t c = 0; c < MPARAM_SSO_HASHTABLE_COLS; c++) {
            struct string *string = I->sso.table[r][c];
            if (string != NULL && mark_object(I, objectI_cast(string))) {
                marked = true;
            }
        }
    }
#else
    (void) I;
#endif

    return marked;
}

static inline bool mark_sio(morphine_instance_t I) {
    bool marked = false;

    if (mark_object(I, objectI_cast(I->sio.io))) {
        marked = true;
    }

    if (mark_object(I, objectI_cast(I->sio.error))) {
        marked = true;
    }

    return marked;
}

static inline bool mark_metatable(morphine_instance_t I) {
    bool marked = false;

    for (enum metatable_field mf = MFS_START; mf < MFS_COUNT; mf++) {
        if (mark_object(I, objectI_cast(I->metatable.names[mf]))) {
            marked = true;
        }
    }

    for (enum value_type type = VALUE_TYPES_START; type < VALUE_TYPES_COUNT; type++) {
        struct table *table = I->metatable.defaults[type];
        if (table != NULL && mark_object(I, objectI_cast(table))) {
            marked = true;
        }
    }

    return marked;
}

static inline bool mark_libraries(morphine_instance_t I) {
    bool marked = false;
    for (size_t i = 0; i < I->libraries.size; i++) {
        struct library *library = I->libraries.array + i;
        if (library->table != NULL && mark_object(I, objectI_cast(library->table))) {
            marked = true;
        }
    }

    return marked;
}

static inline bool mark_throw(morphine_instance_t I) {
    if (!I->E.throw.is_message) {
        return mark_value(I, I->E.throw.error.value);
    }

    return false;
}

static inline bool mark(morphine_instance_t I) {
    bool sso_marked = mark_sso(I);
    bool sio_marked = mark_sio(I);
    bool metatable_marked = mark_metatable(I);
    bool libraries_marked = mark_libraries(I);
    bool throw_marked = mark_throw(I);

    return sso_marked ||
           sio_marked ||
           metatable_marked ||
           libraries_marked ||
           throw_marked;
}

void gcstageI_resolve(morphine_instance_t I, bool emergency) {
    if (mark(I)) {
        while (gcstageI_increment(I, SIZE_MAX)) { }
    }

    if (finalize(I)) {
        while (gcstageI_increment(I, SIZE_MAX)) { }
    }

    attach_black_coroutines(I);

    resolve_pools(I);
    resolve_cache(I, emergency);

    gcI_pools_merge(&I->G.pools.allocated, &I->G.pools.sweep);
    I->G.pools.allocated = I->G.pools.black;
    I->G.pools.black = NULL;

    I->G.stats.debt = 0;
    I->G.stats.prev_allocated = I->G.bytes.allocated;
}
