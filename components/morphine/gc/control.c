//
// Created by whyiskra on 3/23/24.
//

#include "morphine/gc/control.h"
#include "morphine/gc/safe.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"
#include "stages.h"

static inline bool gc_need(morphine_instance_t I, size_t reserved) {
    if (reserved > (SIZE_MAX - I->G.bytes.allocated)) {
        return true;
    }

    size_t alloc_bytes = I->G.bytes.allocated + reserved;

    size_t prev = I->G.bytes.prev_allocated;

    size_t prevdiv = (prev / 100);
    if (unlikely(prevdiv == 0)) {
        prevdiv = 1;
    }

    size_t percent = alloc_bytes / prevdiv;
    bool start = percent >= I->G.settings.grow;
    return start || (alloc_bytes >= I->G.settings.limit_bytes);
}

static inline void ofm_check(morphine_instance_t I, size_t reserved) {
    if (reserved > (SIZE_MAX - I->G.bytes.allocated) || (I->G.bytes.allocated + reserved) >= I->G.settings.limit_bytes) {
        throwI_error(I, "Out of memory");
    }
}

static inline void step(morphine_instance_t I, size_t reserved) {
    if (reserved > (SIZE_MAX - I->G.bytes.allocated) || (I->G.bytes.allocated + reserved) >= I->G.settings.limit_bytes) {
        gcI_full(I, reserved);
        return;
    }

    bool throw_inited = I->E.throw.inited;
    I->E.throw.inited = false;

    while (true) {
        switch (I->G.status) {
            case GC_STATUS_IDLE: {
                I->G.status = GC_STATUS_PREPARE;
                break;
            }
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
        }
    }

exit:
    I->E.throw.inited = throw_inited;
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

void gcI_work(morphine_instance_t I, size_t reserved) {
    if (!I->G.enabled) {
        ofm_check(I, reserved);
        return;
    }

    if (I->G.status != GC_STATUS_IDLE) {
        step(I, reserved);
        return;
    }

    if (gc_need(I, reserved)) {
        step(I, reserved);
    }
}

void gcI_full(morphine_instance_t I, size_t reserved) {
    bool throw_inited = I->E.throw.inited;
    I->E.throw.inited = false;

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

    I->G.status = GC_STATUS_IDLE;

    I->E.throw.inited = throw_inited;

    ofm_check(I, reserved);
}
