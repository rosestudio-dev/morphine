//
// Created by whyiskra on 3/23/24.
//

#pragma once

#include "morphine/core/value.h"
#include "morphine/gc/pools.h"

#define gcI_valbarrier(I, c, v) (gcI_value_barrier((I), objectI_cast(c), (v)))
#define gcI_objbarrier(I, c, o) ((__typeof__(o)) gcI_object_barrier((I), objectI_cast(c), objectI_cast(o)))

static inline struct object *gcI_object_barrier(morphine_instance_t I, struct object *container, struct object *object) {
    if (object == NULL) {
        return object;
    }

    bool need_move = I->G.status == GC_STATUS_INCREMENT
                     && (container->color == OBJ_COLOR_BLACK || container->color == OBJ_COLOR_RED)
                     && object->color == OBJ_COLOR_WHITE;

    if (need_move) {
        object->color = OBJ_COLOR_GREY;
        gcI_pools_remove(object, &I->G.pools.allocated);
        gcI_pools_insert(object, &I->G.pools.grey);
    }

    return object;
}

static inline struct value gcI_value_barrier(morphine_instance_t I, struct object *container, struct value value) {
    if (valueI_is_object(value)) {
        gcI_object_barrier(I, container, valueI_as_object(value));
    }

    return value;
}
