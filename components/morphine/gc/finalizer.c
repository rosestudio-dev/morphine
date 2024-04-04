//
// Created by whyiskra on 3/23/24.
//

#include "morphine/gc/finalizer.h"
#include "morphine/core/instance.h"
#include "morphine/object/native.h"
#include "morphine/stack/call.h"

static void finalizer(morphine_state_t S) {
    morphine_instance_t I = S->I;

    if (callstackI_state(S) == 0) {
        callstackI_continue(S, 0);

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

        callstackI_continue(S, 1);

        I->G.finalizer.candidate = candidate;

        struct value candidate_value = valueI_object(candidate);
        struct value callable = valueI_nil;
        candidate->flags.finalized = true;
        if (metatableI_test(S->I, candidate_value, MF_GC, &callable)) {
            callstackI_unsafe(S, callable, candidate_value, NULL, 0, 0);
            return;
        }
    }

    if (callstackI_state(S) == 1) {
        callstackI_continue(S, 0);

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
    morphine_state_t state = stateI_custom_create(
        I,
        I->settings.finalizer.stack_limit,
        I->settings.finalizer.stack_grow
    );

    stateI_priority(state, 1);

    struct native *native = nativeI_create(I, "gc_finalizer", finalizer);
    callstackI_unsafe(state, valueI_object(native), valueI_nil, NULL, 0, 0);

    I->G.finalizer.state = state;
}