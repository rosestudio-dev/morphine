//
// Created by whyiskra on 28.12.23.
//

#include "morphine/api.h"
#include "morphine/object/state.h"
#include "morphine/core/instance.h"
#include "morphine/gc/control.h"
#include "morphine/gc/status.h"

MORPHINE_API void mapi_gc_full(morphine_instance_t I) {
    gcI_full(I);
}

MORPHINE_API void mapi_gc_work(morphine_instance_t I) {
    gcI_work(I);
}

MORPHINE_API void mapi_gc_force(morphine_instance_t I) {
    gcI_force(I);
}

MORPHINE_API bool mapi_gc_isrunning(morphine_instance_t I) {
    return gcI_isrunning(I);
}

MORPHINE_API void mapi_gc_enable(morphine_instance_t I) {
    gcI_enable(I);
}

MORPHINE_API void mapi_gc_disable(morphine_instance_t I) {
    gcI_disable(I);
}

MORPHINE_API bool mapi_gc_isenabled(morphine_instance_t I) {
    return gcI_isenabled(I);
}

MORPHINE_API const char *mapi_gc_status(morphine_instance_t I) {
    return gcI_status(I);
}

MORPHINE_API size_t mapi_gc_max_allocated(morphine_instance_t I) {
    return I->G.bytes.max_allocated;
}

MORPHINE_API size_t mapi_gc_allocated(morphine_instance_t I) {
    return I->G.bytes.allocated;
}

MORPHINE_API void mapi_gc_change_start(morphine_instance_t I, size_t value) {
    I->G.settings.start = value;
}

MORPHINE_API void mapi_gc_change_grow(morphine_instance_t I, size_t value) {
    I->G.settings.grow = value;
}

MORPHINE_API void mapi_gc_change_deal(morphine_instance_t I, size_t value) {
    I->G.settings.deal = value;
}

MORPHINE_API void mapi_gc_change_finalizer_stack_limit(morphine_instance_t I, size_t value) {
    if (I->G.finalizer.state != NULL) {
        I->G.finalizer.state->settings.stack_limit = value;
    }
}

MORPHINE_API void mapi_gc_change_finalizer_stack_grow(morphine_instance_t I, size_t value) {
    if (I->G.finalizer.state != NULL) {
        I->G.finalizer.state->settings.stack_grow = value;
    }
}

MORPHINE_API void mapi_gc_change_stack_limit(morphine_state_t S, size_t value) {
    S->settings.stack_limit = value;
}

MORPHINE_API void mapi_gc_change_stack_grow(morphine_state_t S, size_t value) {
    S->settings.stack_grow = value;
}
