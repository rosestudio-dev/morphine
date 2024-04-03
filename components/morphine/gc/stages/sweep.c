//
// Created by whyiskra on 3/23/24.
//

#include "../stages.h"
#include "mark.h"
#include "morphine/core/instance.h"
#include "morphine/stack/control.h"
#include "morphine/stack/call.h"
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
        morphine_state_t current = I->states;
        while (current != NULL) {
            stackI_shrink(current);
            current = current->prev;
        }
    }

    {
        struct callinfo *current = I->G.callinfo_trash;
        while (current != NULL) {
            struct callinfo *prev = current->prev;
            callstackI_info_free(I, current);

            current = prev;
        }

        I->G.callinfo_trash = NULL;
    }
}

void gcstageI_sweep(morphine_instance_t I) {
    struct garbage_collector *G = &I->G;

    resolve_refs(I);

    struct object *current = G->pools.allocated;
    while (current != NULL) {
        struct object *prev = current->prev;

        objectI_free(I, current);

        current = prev;
    }

    G->pools.allocated = G->pools.white;
    G->pools.white = NULL;

    shrink(I);

    if (G->bytes.allocated > G->settings.threshold) {
        G->bytes.prev_allocated = G->bytes.allocated;
    } else {
        G->bytes.prev_allocated = G->settings.threshold;
    }
}