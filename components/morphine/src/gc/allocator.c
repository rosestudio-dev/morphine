//
// Created by whyiskra on 16.12.23.
//

#include "morphine/core/throw.h"
#include "morphine/core/instance.h"
#include "morphine/gc/control.h"
#include "morphine/gc/allocator.h"

struct metadata {
    size_t size;
};

static inline void change_allocated_size(morphine_instance_t I, size_t grow_size, bool plus) {
    size_t alloc = I->G.bytes.allocated;

    if (plus) {
        if ((SIZE_MAX - alloc) >= grow_size) {
            I->G.bytes.allocated += grow_size;
        } else {
            throwI_panic(I, "Allocation size overflow");
        }
    } else {
        if (alloc >= grow_size) {
            I->G.bytes.allocated -= grow_size;
        } else {
            throwI_panic(I, "Allocation size corrupted");
        }
    }
}

static inline void calculate_max_alloc_size(morphine_instance_t I) {
    if (I->G.bytes.max_allocated < I->G.bytes.allocated) {
        I->G.bytes.max_allocated = I->G.bytes.allocated;
    }
}

static inline size_t safe_mul(morphine_instance_t I, size_t a, size_t b) {
    if (a > SIZE_MAX / b) {
        throwI_error(I, "Allocation overflow");
    }

    return a * b;
}

static inline size_t safe_add(morphine_instance_t I, size_t a, size_t b) {
    if (a > SIZE_MAX - b) {
        throwI_error(I, "Allocation overflow");
    }

    return a + b;
}

void *allocI_vec(morphine_instance_t I, void *p, size_t n, size_t size) {
    return allocI_uni(I, p, safe_mul(I, n, size));
}

void *allocI_uni(morphine_instance_t I, void *p, size_t nsize) {
    if (likely(nsize == 0)) {
        allocI_free(I, p);
        return NULL;
    }

    nsize = safe_add(I, nsize, sizeof(struct metadata));

    struct metadata *result;
    if (likely(p == NULL)) {
        gcI_work(I, nsize);

        change_allocated_size(I, nsize, true);
        result = I->platform.functions.malloc(nsize);
    } else {
        struct metadata *metadata = p - sizeof(struct metadata);

        if (nsize > metadata->size) {
            size_t reserved = nsize - metadata->size;
            gcI_work(I, reserved);

            change_allocated_size(I, reserved, true);
        } else {
            change_allocated_size(I, metadata->size - nsize, false);
        }

        result = I->platform.functions.realloc(metadata, nsize);
    }

    if (unlikely(result == NULL)) {
        throwI_panic(I, "Allocation fault");
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
