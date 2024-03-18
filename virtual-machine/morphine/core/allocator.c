//
// Created by whyiskra on 16.12.23.
//

#include "morphine/core/allocator.h"
#include "morphine/core/throw.h"
#include "morphine/core/instance.h"

struct pointer {
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
    if (morphinem_likely(nsize == 0)) {
        allocI_free(I, p);
        return NULL;
    }

    nsize += sizeof(struct pointer);

    struct pointer *result;
    if (morphinem_likely(p == NULL)) {
        change_allocated_size(I, nsize, true);
        gcI_work(I);

        result = I->platform.functions.malloc(nsize);
    } else {
        struct pointer *pointer = p - sizeof(struct pointer);

        if (nsize > pointer->size) {
            change_allocated_size(I, nsize - pointer->size, true);
            gcI_work(I);
        } else {
            change_allocated_size(I, pointer->size - nsize, false);
        }

        result = I->platform.functions.realloc(pointer, nsize);
    }

    if (morphinem_unlikely(result == NULL)) {
        throwI_message_panic(I, NULL, "Allocation fault");
    }

    calculate_max_alloc_size(I);

    result->size = nsize;

    return ((void *) result) + sizeof(struct pointer);
}

void allocI_free(morphine_instance_t I, void *p) {
    if (morphinem_likely(p != NULL)) {
        struct pointer *pointer = p - sizeof(struct pointer);
        change_allocated_size(I, pointer->size, false);
        I->platform.functions.free(pointer);
    }
}
