//
// Created by whyiskra on 3/23/24.
//

#include "morphine/gc/control.h"
#include "morphine/gc/safe.h"
#include "morphine/gc/pools.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"
#include "morphine/utils/overflow.h"
#include "stages/impl.h"

static inline bool gc_need(morphine_instance_t I, size_t reserved) {
    overflow_add(reserved, I->G.bytes.allocated, SIZE_MAX) {
        return true;
    }

    size_t alloc_bytes = I->G.bytes.allocated + reserved;

    uintmax_t percent_a = (uintmax_t) alloc_bytes;
    overflow_mul(percent_a, 10, UINTMAX_MAX) {
        return true;
    }

    uintmax_t percent_b = (uintmax_t) I->G.stats.prev_allocated;
    if (unlikely(percent_b == 0)) {
        percent_b = 1;
    }

    uintmax_t percent = (percent_a * 10) / percent_b;
    bool start = percent >= I->G.settings.grow;
    return start || (alloc_bytes >= I->G.settings.limit);
}

static inline void ofm_check(morphine_instance_t I, size_t reserved) {
    if (overflow_condition_add(reserved, I->G.bytes.allocated, SIZE_MAX) ||
        (reserved + I->G.bytes.allocated) > I->G.settings.limit) {
        throwI_error(I, "out of memory");
    }
}

static inline bool pause(morphine_instance_t I) {
    if (likely(I->G.bytes.allocated > I->G.stats.prev_allocated)) {
        size_t debt = I->G.bytes.allocated - I->G.stats.prev_allocated;

        overflow_add(debt, I->G.stats.debt, SIZE_MAX) {
            I->G.stats.debt = SIZE_MAX;
        } else {
            I->G.stats.debt += debt;
        }
    }

    I->G.stats.prev_allocated = I->G.bytes.allocated;

    return I->G.stats.debt <= I->G.settings.pause;
}

static inline size_t debt_calc(morphine_instance_t I) {
    size_t conv = I->G.stats.debt;
    if (unlikely(conv == 0)) {
        conv = 1;
    }

    size_t percent = I->G.settings.deal;
    overflow_mul(conv, percent, SIZE_MAX) {
        return SIZE_MAX;
    }

    return (conv * percent) / 10;
}

static inline void step(morphine_instance_t I, size_t reserved) {
    if (overflow_condition_add(reserved, I->G.bytes.allocated, SIZE_MAX) ||
        (reserved + I->G.bytes.allocated) > I->G.settings.limit) {
        gcI_full(I, reserved);
        return;
    }

    bool throw_inited = I->E.throw.inited;
    I->E.throw.inited = false;

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
            if (likely(pause(I))) {
                break;
            }

            size_t debt = debt_calc(I);
            if (likely(gcstageI_increment(I, debt))) {
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
            if (likely(pause(I))) {
                break;
            }

            size_t debt = debt_calc(I);
            if (unlikely(!gcstageI_sweep(I, debt))) {
                I->G.status = GC_STATUS_IDLE;
            }
            break;
        }
    }

    I->E.throw.inited = throw_inited;
}

static inline void recover_pool(morphine_instance_t I, struct object **pool) {
    gcI_pools_merge(pool, &I->G.pools.allocated);
}

static void recover_pools(morphine_instance_t I) {
    recover_pool(I, &I->G.pools.black);
    recover_pool(I, &I->G.pools.black_coroutines);
    recover_pool(I, &I->G.pools.grey);
}

void gcI_change_limit(morphine_instance_t I, size_t value) {
    I->G.settings.limit = value;
}

void gcI_change_threshold(morphine_instance_t I, size_t value) {
    I->G.settings.threshold = value;
}

void gcI_change_grow(morphine_instance_t I, uint16_t value) {
    if (value <= 100) {
        throwI_error(I, "gc grow must be greater than 100");
    }

    I->G.settings.grow = value / 10;
}

void gcI_change_deal(morphine_instance_t I, uint16_t value) {
    if (value <= 100) {
        throwI_error(I, "gc deal must be greater than 100");
    }

    I->G.settings.deal = value / 10;
}

void gcI_change_pause(morphine_instance_t I, uint8_t value) {
    if (value > 31) {
        throwI_error(I, "gc pause must be less than 32");
    }

    I->G.settings.pause = ((uint32_t) 1) << value;
}

void gcI_change_cache_callinfo_holding(morphine_instance_t I, size_t value) {
    I->G.settings.cache_callinfo_holding = value;
}

void gcI_reset_max_allocated(morphine_instance_t I) {
    I->G.bytes.max_allocated = 0;
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

#ifdef MORPHINE_ENABLE_DISTRIBUTED_GC
    if (I->G.status != GC_STATUS_IDLE) {
        step(I, reserved);
        return;
    }

    if (gc_need(I, reserved)) {
        step(I, reserved);
    }
#else
    gcI_full(I, reserved);
#endif
}

void gcI_full(morphine_instance_t I, size_t reserved) {
    bool throw_inited = I->E.throw.inited;
    I->E.throw.inited = false;

    recover_pools(I);
    gcstageI_prepare(I);
    while (gcstageI_increment(I, SIZE_MAX)) { }
    gcstageI_resolve(I, true);
    while (gcstageI_sweep(I, SIZE_MAX)) { }

    I->G.status = GC_STATUS_IDLE;
    I->E.throw.inited = throw_inited;

    ofm_check(I, reserved);
}
