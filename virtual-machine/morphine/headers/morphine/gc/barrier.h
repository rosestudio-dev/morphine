//
// Created by why on 3/23/24.
//

#pragma once

#include "morphine/core/value.h"

#define gcI_barrier(o, x) morphinem_blk_start struct value _a = (x); if(valueI_is_object(_a)) gcI_objbarrier((o), valueI_as_object(_a)); morphinem_blk_end
#define gcI_objbarrier(o, d) gcI_object_barrier(objectI_cast(o), objectI_cast(d))

static inline void gcI_object_barrier(struct object *s, struct object *d) {
    if (!d->flags.mark) {
        d->flags.mark = s->flags.mark;
    }
}

