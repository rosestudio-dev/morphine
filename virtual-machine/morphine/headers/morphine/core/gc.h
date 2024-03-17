//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "value.h"
#include "morphine/utils/semicolon.h"

#define gcI_barrier(o, x) morphinem_blk_start struct value _a = (x); if(valueI_is_object(_a)) gcI_objbarrier((o), valueI_as_object(_a)); morphinem_blk_end
#define gcI_objbarrier(o, d) gcI_object_barrier(objectI_cast(o), objectI_cast(d))

typedef enum {
    GC_STATUS_IDLE,
    GC_STATUS_PREPARE,
    GC_STATUS_INCREMENT,
    GC_STATUS_FINALIZE_PREPARE,
    GC_STATUS_FINALIZE_INCREMENT,
    GC_STATUS_SWEEP,
} gc_status_t;

struct garbage_collector {
    gc_status_t status;
    size_t cycle;

    bool enabled;

    struct {
        size_t limit_bytes;
        size_t start;
        size_t grow;
        size_t deal;
    } settings;

    struct {
        size_t started;
        size_t allocated;
        size_t prev_allocated;
        size_t max_allocated;
    } bytes;

    struct {
        struct object *allocated;
        struct object *gray;
        struct object *white;

        struct object *finalize_candidates;
    } pools;

    struct callinfo *callinfo_trash;

    struct object *finalize_candidate;
    bool finalize;
};

struct garbage_collector gcI_init(struct params, size_t inited_size);

void gcI_enable(morphine_instance_t);
void gcI_disable(morphine_instance_t);
bool gcI_isenabled(morphine_instance_t);
const char *gcI_status(morphine_instance_t);

void gcI_force(morphine_instance_t);
bool gcI_isrunning(morphine_instance_t);

void gcI_recognize_start(morphine_instance_t);

void gcI_dispose_callinfo(morphine_instance_t, struct callinfo *callinfo);
struct callinfo *gcI_get_hot_callinfo(morphine_instance_t);

void gcI_finalizer_state(morphine_instance_t);

void gcI_work(morphine_instance_t);
void gcI_full(morphine_instance_t);

static inline void gcI_object_barrier(struct object *s, struct object *d) {
    if (!d->mark) {
        d->mark = s->mark;
    }
}
