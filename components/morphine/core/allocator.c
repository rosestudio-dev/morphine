//
// Created by whyiskra on 16.12.23.
//

#include "morphine/core/allocator.h"
#include "morphine/core/throw.h"
#include "morphine/core/instance.h"
#include "morphine/gc/control.h"

struct metadata {
    size_t size;
};

static inline void change_allocated_size(morphine_instance_t I, size_t grow_size, bool plus) {
    size_t alloc = I->G.bytes.allocated;
    if (plus) {
        if ((SIZE_MAX - alloc) >= grow_size) {
            I->G.bytes.allocated += grow_size;
            return;
        }
    } else {
        if (alloc >= grow_size) {
            I->G.bytes.allocated -= grow_size;
            return;
        }
    }

    throwI_message_panic(I, NULL, "Allocation size was corrupted");
}

static inline void calculate_max_alloc_size(morphine_instance_t I) {
    if (I->G.bytes.max_allocated < I->G.bytes.allocated) {
        I->G.bytes.max_allocated = I->G.bytes.allocated;
    }
}

void *allocI_uni(morphine_instance_t I, void *p, size_t nsize) {
    if (likely(nsize == 0)) {
        allocI_free(I, p);
        return NULL;
    }

    size_t temp = nsize;
    nsize += sizeof(struct metadata);

    struct metadata *result;
    if (unlikely(temp >= nsize)) {
        throwI_message_panic(I, NULL, "Allocation size is too big");
    } else if (likely(p == NULL)) {
        change_allocated_size(I, nsize, true);
        gcI_work(I);

        result = I->platform.functions.malloc(nsize);
    } else {
        struct metadata *metadata = p - sizeof(struct metadata);

        if (nsize > metadata->size) {
            change_allocated_size(I, nsize - metadata->size, true);
            gcI_work(I);
        } else {
            change_allocated_size(I, metadata->size - nsize, false);
        }

        result = I->platform.functions.realloc(metadata, nsize);
    }

    if (unlikely(result == NULL)) {
        throwI_message_panic(I, NULL, "Allocation fault");
    }

    calculate_max_alloc_size(I);

    result->size = nsize;

    return ((void *) result) + sizeof(struct metadata);
}

void allocI_free(morphine_instance_t I, void *p) {
    if (likely(p != NULL)) {
        struct metadata *metadata = p - sizeof(struct metadata);
        change_allocated_size(I, metadata->size, false);
        I->platform.functions.free(metadata);
    }
}
