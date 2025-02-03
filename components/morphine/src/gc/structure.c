//
// Created by whyiskra on 3/23/24.
//

#include "morphine/gc/structure.h"
#include "morphine/core/instance.h"
#include "morphine/gc/control.h"
#include "morphine/utils/array_size.h"

static void free_objects(morphine_instance_t I, struct object *pool) {
    struct object *current = pool;

    while (current != NULL) {
        struct object *prev = current->prev;
        objectI_free(I, current);
        current = prev;
    }
}

void gcI_prototype(morphine_instance_t I, size_t inited_bytes) {
    I->G = (struct garbage_collector) {
        .status = GC_STATUS_IDLE,
        .enabled = false,

        .stats.debt = 0,
        .stats.prev_allocated = 0,
        .stats.allocated = inited_bytes,
        .stats.max_allocated = inited_bytes,

        .settings.limit = 0,
        .settings.threshold = 0,
        .settings.grow = 0,
        .settings.deal = 0,
        .settings.pause = 0,

        .pools.allocated = NULL,
        .pools.grey = NULL,
        .pools.black = NULL,
        .pools.black_coroutines = NULL,
        .pools.sweep = NULL,
        .pools.finalize = NULL,

        .finalizer.candidate = NULL,
        .finalizer.coroutine = NULL,
        .finalizer.resolver = NULL,
        .finalizer.name = NULL,

        .safe.values.occupied = 0,
        .safe.rollback.occupied = 0,
    };

    for (size_t i = 0; i < array_size(I->G.safe.values.stack); i++) {
        I->G.safe.values.stack[i] = valueI_nil;
    }

    for (size_t i = 0; i < array_size(I->G.safe.rollback.stack); i++) {
        I->G.safe.rollback.stack[i] = 0;
    }

    gcI_set_limit(I, I->settings.gc.limit);
    gcI_set_threshold(I, I->settings.gc.threshold);
    gcI_set_grow(I, I->settings.gc.grow);
    gcI_set_deal(I, I->settings.gc.deal);
    gcI_set_pause(I, I->settings.gc.pause);
}

void gcI_destruct(morphine_instance_t I, struct garbage_collector G) {
    free_objects(I, G.pools.allocated);
    free_objects(I, G.pools.grey);
    free_objects(I, G.pools.black);
    free_objects(I, G.pools.black_coroutines);
    free_objects(I, G.pools.sweep);
    free_objects(I, G.pools.finalize);

    if (G.finalizer.candidate != NULL) {
        objectI_free(I, G.finalizer.candidate);
    }
}
