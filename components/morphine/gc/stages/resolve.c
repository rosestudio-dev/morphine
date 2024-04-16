//
// Created by why-iskra on 16.04.2024.
//

#include "../stages.h"
#include "mark.h"
#include "morphine/core/instance.h"
#include "morphine/object/reference.h"

static inline void invalidate_ref(struct reference *reference) {
    struct object *object = valueI_safe_as_object(reference->value, NULL);

    if (object == NULL) {
        return;
    }

    if (!object->flags.mark) {
        reference->value = valueI_nil;
    }
}

static inline void resolve_refs(morphine_instance_t I) {
    struct object *current = I->G.pools.white;
    while (current != NULL) {
        if (current->type == OBJ_TYPE_REFERENCE) {
            invalidate_ref(cast(struct reference *, current));
        }

        current = current->prev;
    }
}

static inline void shrink(morphine_instance_t I) {
    {
        morphine_coroutine_t current = I->E.coroutines;
        while (current != NULL) {
            stackI_shrink(current);
            current = current->prev;
        }
    }

    {
        struct callinfo *current = I->G.callinfo_trash;
        while (current != NULL) {
            struct callinfo *prev = current->prev;
            callstackI_callinfo_free(I, current);

            current = prev;
        }

        I->G.callinfo_trash = NULL;
    }
}

void gcstageI_resolve(morphine_instance_t I) {
    if (gcstageI_finalize(I)) {
        while (gcstageI_increment(I, SIZE_MAX)) { }
    }

    resolve_refs(I);
    shrink(I);

    I->G.pools.sweep = I->G.pools.allocated;
    I->G.pools.allocated = I->G.pools.white;
    I->G.pools.white = NULL;

    I->G.stats.debt = 0;
    I->G.stats.prev_allocated = I->G.bytes.allocated;
}
