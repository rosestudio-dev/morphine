//
// Created by whyiskra on 3/23/24.
//

#include "morphine/gc/control.h"
#include "morphine/core/instance.h"
#include "morphine/core/hook.h"
#include "morphine/core/throw.h"
#include "internal/stages.h"

static inline bool gc_need(morphine_instance_t I) {
    size_t alloc_bytes = I->G.bytes.allocated;
    size_t prev = I->G.bytes.prev_allocated;

    if (morphinem_unlikely(prev == 0)) {
        prev = 1;
    }

    size_t percent = (100 * alloc_bytes + prev / 2) / prev;

    bool start = percent >= I->G.settings.grow;

    return start || (alloc_bytes >= I->G.settings.limit_bytes);
}

static inline void ofm_check(morphine_instance_t I) {
    if (I->G.bytes.allocated >= I->G.settings.limit_bytes) {
        throwI_message_panic(I, NULL, "Out of memory");
    }
}

static inline void step(morphine_instance_t I) {
    pdbg_hook_gc_step_enter(I);

    if (I->G.bytes.allocated >= I->G.settings.limit_bytes) {
        pdbg_hook_gc_step_exit(I);
        gcI_full(I);
        return;
    }

    while (true) {
        switch (I->G.status) {
            case GC_STATUS_PREPARE: {
                gcstageI_prepare(I);
                I->G.status = GC_STATUS_INCREMENT;
                goto exit;
            }
            case GC_STATUS_INCREMENT: {
                if (gcstageI_increment(I)) {
                    I->G.status = GC_STATUS_FINALIZE_PREPARE;
                    break;
                } else {
                    goto exit;
                }
            }
            case GC_STATUS_FINALIZE_PREPARE: {
                if (gcstageI_finalize(I)) {
                    I->G.status = GC_STATUS_FINALIZE_INCREMENT;
                    goto exit;
                } else {
                    I->G.status = GC_STATUS_SWEEP;
                    break;
                }
            }
            case GC_STATUS_FINALIZE_INCREMENT: {
                if (gcstageI_increment(I)) {
                    I->G.status = GC_STATUS_SWEEP;
                    break;
                } else {
                    goto exit;
                }
            }
            case GC_STATUS_SWEEP: {
                gcstageI_sweep(I);
                I->G.status = GC_STATUS_IDLE;
                goto exit;
            }
            case GC_STATUS_IDLE: {
                goto exit;
            }
        }

        pdbg_hook_gc_step_middle(I);
    }

exit:
    pdbg_hook_gc_step_exit(I);
}

void gcI_recognize(morphine_instance_t I) {
    I->G.bytes.started = I->G.bytes.allocated;
}

void gcI_enable(morphine_instance_t I) {
    I->G.enabled = true;
}

void gcI_disable(morphine_instance_t I) {
    I->G.enabled = false;
}

void gcI_force(morphine_instance_t I) {
    if (I->G.status == GC_STATUS_IDLE) {
        I->G.status = GC_STATUS_PREPARE;
    }
}

void gcI_work(morphine_instance_t I) {
    if (!I->G.enabled) {
        ofm_check(I);
        return;
    }

    if (I->G.status != GC_STATUS_IDLE) {
        step(I);
        return;
    }

    if (gc_need(I)) {
        I->G.status = GC_STATUS_PREPARE;
        step(I);
    }
}

void gcI_full(morphine_instance_t I) {
    pdbg_hook_gc_full_enter(I);

    if (I->G.pools.white != NULL) {
        I->G.pools.white->prev = I->G.pools.allocated;
        I->G.pools.allocated = I->G.pools.white;
    }

    if (I->G.pools.gray != NULL) {
        I->G.pools.gray->prev = I->G.pools.allocated;
        I->G.pools.allocated = I->G.pools.gray;
    }

    gcstageI_prepare(I);
    while (!gcstageI_increment(I)) { }

    if (gcstageI_finalize(I)) {
        while (!gcstageI_increment(I)) { }
    }

    gcstageI_sweep(I);
    ofm_check(I);

    I->G.status = GC_STATUS_IDLE;

    pdbg_hook_gc_full_exit(I);
}
