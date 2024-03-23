//
// Created by why on 3/23/24.
//

#include "morphine/gc/structure.h"

struct garbage_collector gcI_init(struct params params, size_t inited_size) {
    return (struct garbage_collector) {
        .status = GC_STATUS_IDLE,
        .enabled = false,
        .cycle = 1,

        .settings.limit_bytes = params.gc.limit_bytes,
        .settings.start = params.gc.start,
        .settings.grow = params.gc.grow,
        .settings.deal = params.gc.deal,

        .bytes.started = inited_size,
        .bytes.allocated = inited_size,
        .bytes.max_allocated = inited_size,
        .bytes.prev_allocated = 0,

        .pools.allocated = NULL,
        .pools.gray = NULL,
        .pools.white = NULL,
        .pools.finalize = NULL,

        .finalizer.candidate = NULL,
        .finalizer.state = NULL,
        .finalizer.work = false,

        .callinfo_trash = NULL,
    };
}
