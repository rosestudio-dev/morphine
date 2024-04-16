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

    uintmax_t percent_a = (uintmax_t) alloc_bytes;
    if (unlikely(percent_a > UINTMAX_MAX / 10)) {
        return true;
    }

    uintmax_t percent_b = (uintmax_t) I->G.stats.prev_allocated;
    if (unlikely(percent_b == 0)) {
        percent_b = 1;
    }

    uintmax_t percent = (percent_a * 10) / percent_b;
    bool start = percent >= (I->G.settings.grow / 10);
    return start || (alloc_bytes >= I->G.settings.limit_bytes);
}

static inline void ofm_check(morphine_instance_t I, size_t reserved) {
    if (reserved > (SIZE_MAX - I->G.bytes.allocated) ||
        (I->G.bytes.allocated + reserved) >= I->G.settings.limit_bytes) {
        throwI_error(I, "Out of memory");
    }
}

static inline bool pause(morphine_instance_t I) {
    uint32_t size = ((uint32_t) 1) << I->G.settings.pause;
    if (unlikely(I->G.settings.pause == 0)) {
        size = 0;
    } else if (unlikely(I->G.settings.pause > 31)) {
        size = ((uint32_t) 1) << 31;
    }

    return I->G.stats.debt <= size;
}

static inline bool update_debt(morphine_instance_t I) {
    if (likely(I->G.bytes.allocated > I->G.stats.prev_allocated)) {
        size_t debt = I->G.bytes.allocated - I->G.stats.prev_allocated;

        if (likely(debt <= SIZE_MAX - I->G.stats.debt)) {
            I->G.stats.debt += debt;
        } else {
            I->G.stats.debt = SIZE_MAX;
        }
    }

    I->G.stats.prev_allocated = I->G.bytes.allocated;

    return pause(I);
}

static inline size_t debt_calc(morphine_instance_t I) {
    size_t conv = (uintmax_t) I->G.stats.debt;
    if (unlikely(conv == 0)) {
        conv = 1;
    }

    size_t percent = I->G.settings.deal / 10;
    if (unlikely(conv > SIZE_MAX / percent)) {
        return SIZE_MAX;
    }

    return (conv * percent) / 10;
}

static inline void step(morphine_instance_t I, size_t reserved) {
    if (reserved > (SIZE_MAX - I->G.bytes.allocated) ||
        (I->G.bytes.allocated + reserved) >= I->G.settings.limit_bytes) {
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
            if (likely(update_debt(I))) {
                break;
            }

            size_t debt = debt_calc(I);
            if (likely(gcstageI_increment(I, false, debt))) {
                break;
            }

            I->G.status = GC_STATUS_RESOLVE;
            goto resolve;
        }
        case GC_STATUS_RESOLVE: {
resolve:
            gcstageI_resolve(I);
            I->G.status = GC_STATUS_SWEEP;
            break;
        }
        case GC_STATUS_SWEEP: {
            if (likely(update_debt(I))) {
                break;
            }

            size_t debt = debt_calc(I);
            if (unlikely(!gcstageI_sweep(I, false, debt))) {
                I->G.status = GC_STATUS_IDLE;
            }
            break;
        }
    }

    I->E.throw.inited = throw_inited;
}

static inline void recover_pool(morphine_instance_t I, struct object **pool) {
    if (*pool != NULL) {
        struct object *end = NULL;
        struct object *current = *pool;
        while (current != NULL) {
            end = current;
            current = current->prev;
        }

        if (end != NULL) {
            end->prev = I->G.pools.allocated;
            I->G.pools.allocated = *pool;
        }

        *pool = NULL;
    }
}

static void recover_pools(morphine_instance_t I) {
    recover_pool(I, &I->G.pools.sweep);
    recover_pool(I, &I->G.pools.white);
    recover_pool(I, &I->G.pools.gray);
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

    recover_pools(I);
    gcstageI_prepare(I);
    while (gcstageI_increment(I, true, 0)) { }
    gcstageI_resolve(I);
    while (gcstageI_sweep(I, true, 0)) { }

    I->G.status = GC_STATUS_IDLE;
    I->E.throw.inited = throw_inited;

    ofm_check(I, reserved);
}
