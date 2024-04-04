//
// Created by whyiskra on 3/23/24.
//

#include "morphine/gc/structure.h"
#include "morphine/object/coroutine/callstack.h"
#include "morphine/object.h"

static void free_objects(morphine_instance_t I, struct object *pool) {
    struct object *current = pool;

    while (current != NULL) {
        struct object *prev = current->prev;
        objectI_free(I, current);
        current = prev;
    }
}

struct garbage_collector gcI_prototype(struct gc_settings settings, size_t inited_bytes) {
    struct garbage_collector G = {
        .status = GC_STATUS_IDLE,
        .enabled = false,

        .settings = settings,

        .stats.debt = 0,

        .bytes.allocated = inited_bytes,
        .bytes.max_allocated = inited_bytes,
        .bytes.prev_allocated = 0,

        .pools.allocated = NULL,
        .pools.gray = NULL,
        .pools.white = NULL,
        .pools.finalize = NULL,

        .finalizer.candidate = NULL,
        .finalizer.coroutine = NULL,
        .finalizer.work = false,

        .safe.index = 0,

        .callinfo_trash = NULL,
    };

    size_t size = sizeof(G.safe.stack) / sizeof(struct value);

    for (size_t i = 0; i < size; i++) {
        G.safe.stack[i] = valueI_nil;
    }

    return G;
}

void gcI_destruct(morphine_instance_t I, struct garbage_collector G) {
    free_objects(I, G.pools.allocated);
    free_objects(I, G.pools.gray);
    free_objects(I, G.pools.white);
    free_objects(I, G.pools.finalize);

    if (G.finalizer.candidate != NULL) {
        objectI_free(I, G.finalizer.candidate);
    }

    struct callinfo *current = G.callinfo_trash;
    while (current != NULL) {
        struct callinfo *prev = current->prev;
        callstackI_callinfo_free(I, current);

        current = prev;
    }
}
