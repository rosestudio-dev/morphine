//
// Created by why-iskra on 18.04.2024.
//

#pragma once

#include "morphine/core/instance.h"

static inline void gcI_pools_insert(
    struct object *object,
    struct object **pool
) {
    object->next = NULL;
    object->prev = (*pool);

    if ((*pool) != NULL) {
        (*pool)->next = object;
    }

    (*pool) = object;
}

static inline void gcI_pools_remove(
    struct object *object,
    struct object **pool
) {
    if (object->prev != NULL) {
        object->prev->next = object->next;
    }

    if (object->next == NULL) { // head ?
        (*pool) = object->prev;
    } else {
        object->next->prev = object->prev;
    }

    object->next = NULL;
    object->prev = NULL;
}
