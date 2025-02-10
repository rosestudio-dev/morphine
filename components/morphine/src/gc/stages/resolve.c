//
// Created by why-iskra on 16.04.2024.
//

#include "impl.h"
#include "mark.h"
#include "morphine/core/instance.h"

static inline bool value_is_black(struct value value) {
    struct object *object = valueI_as_object_or_default(value, NULL);
    if (object == NULL) {
        return true;
    }

    return object->color == OBJ_COLOR_BLACK;
}

static inline void invalidate_table(morphine_instance_t I, struct table *table) {
    if (table->metatable == NULL) {
        return;
    }

    struct value result;
    bool weak_key = false;
    if (metatableI_test(I, valueI_object(table), MTYPE_METAFIELD_WEAK_KEY, &result)) {
        weak_key = valueI_tobool(result);
    }

    bool weak_val = false;
    if (metatableI_test(I, valueI_object(table), MTYPE_METAFIELD_WEAK_VALUE, &result)) {
        weak_val = valueI_tobool(result);
    }

    if (!weak_val && !weak_key) {
        return;
    }

    struct bucket *current = table->hashmap.buckets.tail;
    while (current != NULL) {
        struct bucket *next = current->ll.next;
        bool remove_by_key = weak_key && !value_is_black(current->pair.key);
        bool remove_by_val = weak_val && !value_is_black(current->pair.value);
        if (remove_by_key || remove_by_val) {
            tableI_idx_remove(I, table, current->ll.index);
        }
        current = next;
    }
}

static inline void invalidate_coroutine(struct coroutine *coroutine, bool emergency) {
    stackI_reduce_stack(coroutine, emergency);
    stackI_reduce_cache(coroutine, emergency);
}

static inline void invalidate(morphine_instance_t I, struct object *object, bool emergency) {
    if (object->type == OBJ_TYPE_TABLE) {
        invalidate_table(I, cast(struct table *, object));
    } else if (object->type == OBJ_TYPE_COROUTINE) {
        invalidate_coroutine(cast(struct coroutine *, object), emergency);
    }
}

static inline void invalidate_pool(morphine_instance_t I, struct object *pool, bool emergency) {
    struct object *current = pool;
    while (current != NULL) {
        invalidate(I, current, emergency);
        current = current->prev;
    }
}

static inline void resolve_pools(morphine_instance_t I, bool emergency) {
    invalidate_pool(I, I->G.pools.black, emergency);
    invalidate_pool(I, I->G.pools.finalize, emergency);

    if (I->G.finalizer.candidate != NULL) {
        invalidate(I, I->G.finalizer.candidate, emergency);
    }
}

static inline void attach_black_coroutines(morphine_instance_t I) {
    struct object *current = I->G.pools.black_coroutines;
    while (current != NULL) {
        gcI_pools_remove(current, &I->G.pools.black_coroutines);
        gcI_pools_insert(current, &I->G.pools.black);

        current = I->G.pools.black_coroutines;
    }
}

static inline bool finalize(morphine_instance_t I) {
    bool has_to_be_finalize = false;
    struct object *current = I->G.pools.allocated;
    while (current != NULL) {
        struct object *prev = current->prev;

        if (mm_unlikely(
                !current->flags.finalized && metatableI_test(I, valueI_object(current), MTYPE_METAFIELD_GC, NULL)
            )) {
            current->color = OBJ_COLOR_RED;
            gcI_pools_remove(current, &I->G.pools.allocated);
            gcI_pools_insert(current, &I->G.pools.finalize);

            has_to_be_finalize = true;
        }

        current = prev;
    }

    return has_to_be_finalize;
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

static inline bool mark_stream(morphine_instance_t I) {
    bool marked = false;

    if (mark_object(I, objectI_cast(I->stream.io))) {
        marked = true;
    }

    if (mark_object(I, objectI_cast(I->stream.err))) {
        marked = true;
    }

    return marked;
}

static inline bool mark_metafields(morphine_instance_t I) {
    bool marked = false;

    for (mtype_metafield_t mf = MORPHINE_METAFIELDS_START; mf < MORPHINE_METAFIELDS_COUNT; mf++) {
        if (mark_object(I, objectI_cast(I->metafields[mf]))) {
            marked = true;
        }
    }

    return marked;
}

static inline bool mark_libraries(morphine_instance_t I) {
    bool marked = false;

    struct library *library = I->libraries.list;
    while (library != NULL) {
        if (library->name != NULL && mark_object(I, objectI_cast(library->name))) {
            marked = true;
        }

        if (library->table != NULL && mark_object(I, objectI_cast(library->table))) {
            marked = true;
        }

        library = library->prev;
    }

    return marked;
}

static inline bool mark_usertypes(morphine_instance_t I) {
    bool marked = false;

    struct usertype *usertype = I->usertypes.list;
    while (usertype != NULL) {
        if (usertype->metatable != NULL && mark_object(I, objectI_cast(usertype->metatable))) {
            marked = true;
        }

        usertype = usertype->prev;
    }

    return marked;
}

static inline bool mark_throw(morphine_instance_t I) {
    bool marked = false;
    if (I->throw.type == THROW_TYPE_VALUE && mark_value(I, I->throw.error.value)) {
        marked = true;
    }

    if (I->throw.special.ofm != NULL && mark_object(I, objectI_cast(I->throw.special.ofm))) {
        marked = true;
    }

    if (I->throw.special.af != NULL && mark_object(I, objectI_cast(I->throw.special.af))) {
        marked = true;
    }

    return marked;
}

static inline bool mark_gc(morphine_instance_t I) {
    bool marked = false;
    if (I->G.finalizer.resolver != NULL && mark_object(I, objectI_cast(I->G.finalizer.resolver))) {
        marked = true;
    }

    if (I->G.finalizer.name != NULL && mark_object(I, objectI_cast(I->G.finalizer.name))) {
        marked = true;
    }

    return marked;
}

static inline bool mark(morphine_instance_t I) {
    bool sso_marked = mark_sso(I);
    bool stream_marked = mark_stream(I);
    bool metafields_marked = mark_metafields(I);
    bool libraries_marked = mark_libraries(I);
    bool usertypes_marked = mark_usertypes(I);
    bool throw_marked = mark_throw(I);
    bool gc_marked = mark_gc(I);

    return sso_marked || stream_marked || metafields_marked || usertypes_marked || libraries_marked || throw_marked
           || gc_marked;
}

void gcstageI_resolve(morphine_instance_t I, bool emergency) {
    if (mark(I)) {
        while (gcstageI_increment(I, SIZE_MAX)) { }
    }

    if (finalize(I)) {
        while (gcstageI_increment(I, SIZE_MAX)) { }
    }

    attach_black_coroutines(I);

    resolve_pools(I, emergency);

    gcI_pools_merge(&I->G.pools.allocated, &I->G.pools.sweep);
    I->G.pools.allocated = I->G.pools.black;
    I->G.pools.black = NULL;

    I->G.stats.debt = 0;
    I->G.stats.prev_allocated = I->G.stats.allocated;
}
