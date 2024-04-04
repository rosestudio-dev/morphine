//
// Created by whyiskra on 3/23/24.
//

#include "morphine/gc/finalizer.h"
#include "morphine/core/instance.h"
#include "morphine/object/native.h"
#include "morphine/stack/call.h"

static void finalizer(morphine_coroutine_t U) {
    morphine_instance_t I = U->I;

    if (callstackI_state(U) == 0) {
        callstackI_continue(U, 0);

        struct object *candidate = I->G.pools.finalize;
        if (candidate == NULL) {
            I->G.finalizer.work = false;
            return;
        }

        I->G.pools.finalize = candidate->prev;

        if (candidate->flags.mark || candidate->flags.finalized) {
            candidate->prev = I->G.pools.allocated;
            I->G.pools.allocated = candidate;
            return;
        }

        callstackI_continue(U, 1);

        I->G.finalizer.candidate = candidate;

        struct value candidate_value = valueI_object(candidate);
        struct value callable = valueI_nil;
        candidate->flags.finalized = true;
        if (metatableI_test(U->I, candidate_value, MF_GC, &callable)) {
            callstackI_unsafe(U, callable, candidate_value, NULL, 0, 0);
            return;
        }
    }

    if (callstackI_state(U) == 1) {
        callstackI_continue(U, 0);

        struct object *candidate = I->G.finalizer.candidate;
        I->G.finalizer.candidate = NULL;

        if (candidate != NULL) {
            candidate->prev = I->G.pools.allocated;
            I->G.pools.allocated = candidate;
        }

        return;
    }

    throwI_panic(I, "Undefined gc finalizer's state");
}

void gcI_init_finalizer(morphine_instance_t I) {
    morphine_coroutine_t coroutine = coroutineI_custom_create(
        I,
        I->settings.finalizer.stack_limit,
        I->settings.finalizer.stack_grow
    );

    coroutineI_priority(coroutine, 1);

    struct native *native = nativeI_create(I, "gc_finalizer", finalizer);
    callstackI_unsafe(coroutine, valueI_object(native), valueI_nil, NULL, 0, 0);

    I->G.finalizer.coroutine = coroutine;
}