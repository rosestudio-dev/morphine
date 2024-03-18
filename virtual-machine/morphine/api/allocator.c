//
// Created by why on 3/19/24.
//

#include "morphine/api.h"
#include "morphine/core/allocator.h"

MORPHINE_API void *mapi_allocator_uni(morphine_instance_t I, void *p, size_t size) {
    return allocI_uni(I, p, size);
}

MORPHINE_API void mapi_allocator_free(morphine_instance_t I, void *p) {
    allocI_free(I, p);
}
