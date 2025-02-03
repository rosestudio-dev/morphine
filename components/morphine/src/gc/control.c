//
// Created by whyiskra on 3/23/24.
//

#include "morphine/gc/control.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"
#include "morphine/gc/pools.h"
#include "morphine/gc/safe.h"
#include "morphine/utils/assert.h"
#include "morphine/utils/overflow.h"
#include "stages/impl.h"

static inline bool gc_need(morphine_instance_t I, size_t reserved) {
    size_t alloc_bytes = mm_overflow_opc_add(I->G.stats.allocated, reserved, return true);

    uintmax_t percent_a = mm_overflow_opc_mul((uintmax_t) alloc_bytes, 100, return true);
    uintmax_t percent_b = I->G.stats.prev_allocated == 0 ? 1 : (uintmax_t) I->G.stats.prev_allocated;
    uintmax_t percent = percent_a / percent_b;

    bool start = percent >= I->G.settings.grow;
    return start || (alloc_bytes >= I->G.settings.limit);
}

static inline bool pause(morphine_instance_t I) {
    if (mm_likely(I->G.stats.allocated > I->G.stats.prev_allocated)) {
        size_t debt = I->G.stats.allocated - I->G.stats.prev_allocated;
        I->G.stats.debt = mm_overflow_opd_add(I->G.stats.debt, debt, SIZE_MAX);
    }

    I->G.stats.prev_allocated = I->G.stats.allocated;

    return I->G.stats.debt <= I->G.settings.pause;
}

static inline size_t debt_calc(morphine_instance_t I) {
    return mm_overflow_opc_mul(I->G.stats.debt, I->G.settings.deal, return SIZE_MAX) / 100;
}

static inline void step(morphine_instance_t I) {
    throwI_danger_enter(I);

    switch (I->G.status) {
        case GC_STATUS_IDLE: {
            I->G.status = GC_STATUS_PREPARE;
            break;
        }
        case GC_STATUS_PREPARE: {
            gcstageI_prepare(I);
            I->G.status = GC_STATUS_INCREMENT;
            break;
        }
        case GC_STATUS_INCREMENT: {
            if (mm_likely(pause(I))) {
                break;
            }

            size_t debt = debt_calc(I);
            if (mm_likely(gcstageI_increment(I, debt))) {
                break;
            }

            I->G.status = GC_STATUS_RESOLVE;
            goto resolve;
        }
        case GC_STATUS_RESOLVE: {
resolve:
            gcstageI_resolve(I, false);
            I->G.status = GC_STATUS_SWEEP;
            break;
        }
        case GC_STATUS_SWEEP: {
            if (mm_likely(pause(I))) {
                break;
            }

            size_t debt = debt_calc(I);
            if (mm_unlikely(!gcstageI_sweep(I, debt))) {
                I->G.status = GC_STATUS_IDLE;
            }
            break;
        }
    }

    throwI_danger_exit(I);
}

static inline void recover_pool(morphine_instance_t I, struct object **pool) {
    gcI_pools_merge(pool, &I->G.pools.allocated);
}

static void recover_pools(morphine_instance_t I) {
    recover_pool(I, &I->G.pools.black);
    recover_pool(I, &I->G.pools.black_coroutines);
    recover_pool(I, &I->G.pools.grey);
}

void gcI_set_limit(morphine_instance_t I, size_t value) {
    if (mm_overflow_cond_cast(ml_integer, value)) {
        throwI_error(I, "gc limit is too large");
    }

    I->G.settings.limit = value;
}

void gcI_set_threshold(morphine_instance_t I, size_t value) {
    I->G.settings.threshold = value;
}

void gcI_set_grow(morphine_instance_t I, size_t value) {
    if (value <= 100) {
        throwI_error(I, "gc grow must be greater than 100");
    }

    I->G.settings.grow = value;
}

void gcI_set_deal(morphine_instance_t I, size_t value) {
    if (value <= 100) {
        throwI_error(I, "gc deal must be greater than 100");
    }

    I->G.settings.deal = value;
}

void gcI_set_pause(morphine_instance_t I, size_t value) {
    I->G.settings.pause = value;
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
    size_t allocated = I->G.stats.allocated;
    if (!I->G.enabled) {
        goto exit;
    }

#ifdef MORPHINE_ENABLE_DISTRIBUTED_GC
    if (I->G.status != GC_STATUS_IDLE) {
        step(I);
        goto exit;
    }

    if (gc_need(I, reserved)) {
        step(I);
    }
#else
    (void) gc_need;
    (void) step;
    (void) reserved;
    gcI_full(I);
#endif
exit:
    mm_assert(I, allocated >= I->G.stats.allocated, "allocation growing after gc work");
}

void gcI_full(morphine_instance_t I) {
    size_t allocated = I->G.stats.allocated;

    throwI_danger_enter(I);

    recover_pools(I);
    gcstageI_prepare(I);
    while (gcstageI_increment(I, SIZE_MAX)) { }
    gcstageI_resolve(I, true);
    while (gcstageI_sweep(I, SIZE_MAX)) { }

    I->G.status = GC_STATUS_IDLE;
    throwI_danger_exit(I);

    mm_assert(I, allocated >= I->G.stats.allocated, "allocation growing after gc full work");
}
