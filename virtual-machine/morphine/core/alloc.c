//
// Created by whyiskra on 16.12.23.
//

#include "morphine/core/alloc.h"
#include "morphine/core/throw.h"
#include "morphine/core/instance.h"

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

void *allocI_uni(morphine_instance_t I, void *p, size_t osize, size_t nsize) {
    if (morphinem_likely(p != NULL && nsize == 0)) {
        change_allocated_size(I, osize, false);
        I->platform.functions.free(p);
        return NULL;
    }

    if (p != NULL) {
        if (nsize > osize) {
            change_allocated_size(I, nsize - osize, true);
            gcI_work(I);
        } else {
            change_allocated_size(I, osize - nsize, false);
        }

        void *result = I->platform.functions.realloc(p, nsize);

        calculate_max_alloc_size(I);

        if (result == NULL) {
            throwI_message_panic(I, NULL, "Allocation fault");
        }

        return result;
    }

    if (nsize > 0) {
        change_allocated_size(I, nsize, true);
        gcI_work(I);

        void *result = I->platform.functions.malloc(nsize);

        calculate_max_alloc_size(I);

        if (result == NULL) {
            throwI_message_panic(I, NULL, "Allocation fault");
        }

        return result;
    }

    return NULL;
}
