//
// Created by whyiskra on 28.12.23.
//

#include "morphine/api.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/instance.h"
#include "morphine/gc/control.h"
#include "morphine/gc/status.h"

MORPHINE_API void mapi_gc_full(morphine_instance_t I) {
    gcI_full(I);
}

MORPHINE_API void mapi_gc_work(morphine_instance_t I) {
    gcI_work(I, 0);
}

MORPHINE_API void mapi_gc_force(morphine_instance_t I) {
    gcI_force(I);
}

MORPHINE_API bool mapi_gc_is_running(morphine_instance_t I) {
    return gcI_isrunning(I);
}

MORPHINE_API void mapi_gc_enable(morphine_instance_t I) {
    gcI_enable(I);
}

MORPHINE_API void mapi_gc_disable(morphine_instance_t I) {
    gcI_disable(I);
}

MORPHINE_API bool mapi_gc_is_enabled(morphine_instance_t I) {
    return gcI_isenabled(I);
}

MORPHINE_API const char *mapi_gc_status(morphine_instance_t I) {
    return gcI_status(I);
}

MORPHINE_API size_t mapi_gc_max_allocated(morphine_instance_t I) {
    return I->G.stats.max_allocated;
}

MORPHINE_API size_t mapi_gc_allocated(morphine_instance_t I) {
    return I->G.stats.allocated;
}

MORPHINE_API void mapi_gc_set_limit(morphine_instance_t I, size_t value) {
    gcI_set_limit(I, value);
}

MORPHINE_API void mapi_gc_set_threshold(morphine_instance_t I, size_t value) {
    gcI_set_threshold(I, value);
}

MORPHINE_API void mapi_gc_set_grow(morphine_instance_t I, size_t value) {
    gcI_set_grow(I, value);
}

MORPHINE_API void mapi_gc_set_deal(morphine_instance_t I, size_t value) {
    gcI_set_deal(I, value);
}

MORPHINE_API void mapi_gc_set_pause(morphine_instance_t I, size_t value) {
    gcI_set_pause(I, value);
}
