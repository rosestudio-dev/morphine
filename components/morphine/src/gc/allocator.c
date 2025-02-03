//
// Created by whyiskra on 16.12.23.
//

#include "morphine/gc/allocator.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"
#include "morphine/gc/control.h"
#include "morphine/utils/overflow.h"

struct metadata {
    size_t size;
};

static inline bool ofm_condition(morphine_instance_t I, size_t reserved) {
    return mm_overflow_cond_add(I->G.stats.allocated, reserved)
           || (I->G.stats.allocated + reserved) > I->G.settings.limit;
}

static inline void increase_allocated(morphine_instance_t I, size_t size) {
    if (ofm_condition(I, size)) {
        gcI_full(I);
    } else {
        gcI_work(I, size);
    }

    if (ofm_condition(I, size)) {
        throwI_ofm(I);
    }

    I->G.stats.allocated += size;
}

static inline void decrease_allocated(morphine_instance_t I, size_t size) {
    I->G.stats.allocated = mm_overflow_opc_sub(I->G.stats.allocated, size, throwI_panic(I, "allocation was corrupted"));
}

void *allocI_vec(morphine_instance_t I, void *p, size_t n, size_t size) {
    size_t bytes = mm_overflow_opc_mul(n, size, throwI_error(I, "allocation size is too large"));
    return allocI_uni(I, p, bytes);
}

void *allocI_uni(morphine_instance_t I, void *p, size_t nsize) {
    if (mm_likely(nsize == 0)) {
        allocI_free(I, p);
        return NULL;
    }

    size_t metadata_size = sizeof(struct metadata);
    nsize = mm_overflow_opc_add(nsize, metadata_size, throwI_error(I, "allocation size is too large"));

    struct metadata *result;
    if (p == NULL) {
        increase_allocated(I, nsize);
        result = I->platform.memory.alloc(I->data, nsize);
    } else {
        struct metadata *metadata = p - sizeof(struct metadata);

        if (nsize > metadata->size) {
            increase_allocated(I, nsize - metadata->size);
        } else {
            decrease_allocated(I, metadata->size - nsize);
        }

        result = I->platform.memory.realloc(I->data, metadata, nsize);
    }

    if (mm_unlikely(result == NULL)) {
        throwI_af(I);
    }

    result->size = nsize;

    if (I->G.stats.max_allocated < I->G.stats.allocated) {
        I->G.stats.max_allocated = I->G.stats.allocated;
    }

    return ((void *) result) + sizeof(struct metadata);
}

void allocI_free(morphine_instance_t I, void *p) {
    if (p == NULL) {
        return;
    }

    struct metadata *metadata = p - sizeof(struct metadata);
    decrease_allocated(I, metadata->size);
    I->platform.memory.free(I->data, metadata);
}
