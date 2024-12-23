//
// Created by whyiskra on 3/23/24.
//

#pragma once

#include "morphine/core/value.h"
#include "morphine/gc/pools.h"

#define gcI_barrier(I, o, x)    do { struct value _a = (x); if(valueI_is_object(_a)) gcI_objbarrier((I), (o), valueI_as_object(_a)); } while (0)
#define gcI_objbarrier(I, o, d) gcI_object_barrier((I), objectI_cast(o), objectI_cast(d))

static inline void gcI_object_barrier(
    morphine_instance_t I,
    struct object *container,
    struct object *object
) {
    if (object == NULL) {
        return;
    }

    bool need_move = I->G.status == GC_STATUS_INCREMENT &&
                     (container->color == OBJ_COLOR_BLACK || container->color == OBJ_COLOR_RED) &&
                     object->color == OBJ_COLOR_WHITE;

    if (need_move) {
        object->color = OBJ_COLOR_GREY;
        gcI_pools_remove(object, &I->G.pools.allocated);
        gcI_pools_insert(object, &I->G.pools.grey);
    }
}

