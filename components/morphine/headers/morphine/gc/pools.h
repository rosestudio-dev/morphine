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

static inline void gcI_pools_merge(
    struct object **from,
    struct object **pool
) {
    struct object *object = *from;
    if (*pool == NULL) {
        *pool = object;
    } else if(object != NULL) {
        struct object *last = *pool;
        while (last->prev != NULL) {
            last = last->prev;
        }

        last->prev = object;
        object->next = last;
    }

    *from = NULL;
}
