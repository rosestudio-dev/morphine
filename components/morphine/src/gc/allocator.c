//
// Created by whyiskra on 16.12.23.
//

#include "morphine/core/throw.h"
#include "morphine/core/instance.h"
#include "morphine/gc/control.h"
#include "morphine/gc/allocator.h"
#include "morphine/utils/overflow.h"

struct metadata {
    size_t size;
};

static inline void change_allocated_size(morphine_instance_t I, size_t grow_size, bool plus) {
    size_t alloc = I->G.bytes.allocated;

    if (plus) {
        overflow_add(alloc, grow_size, SIZE_MAX) {
            throwI_panic(I, "allocation size overflow");
        }

        I->G.bytes.allocated += grow_size;
    } else {
        overflow_sub(alloc, grow_size, 0) {
            throwI_panic(I, "allocation size corrupted");
        }

        I->G.bytes.allocated -= grow_size;
    }
}

static inline void update_max_alloc_size(morphine_instance_t I) {
    if (I->G.bytes.max_allocated < I->G.bytes.allocated) {
        I->G.bytes.max_allocated = I->G.bytes.allocated;
    }
}

static inline size_t safe_mul(morphine_instance_t I, size_t a, size_t b) {
    overflow_mul(a, b, SIZE_MAX) {
        throwI_error(I, "allocation overflow");
    }

    return a * b;
}

static inline size_t safe_add(morphine_instance_t I, size_t a, size_t b) {
    overflow_add(a, b, SIZE_MAX) {
        throwI_error(I, "allocation overflow");
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
    if (p == NULL) {
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
        throwI_panic(I, "allocation fault");
    }

    update_max_alloc_size(I);

    result->size = nsize;

    return ((void *) result) + sizeof(struct metadata);
}

void allocI_free(morphine_instance_t I, void *p) {
    if (p == NULL) {
        return;
    }

    struct metadata *metadata = p - sizeof(struct metadata);
    change_allocated_size(I, metadata->size, false);
    I->platform.functions.free(metadata);
}
