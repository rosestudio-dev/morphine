//
// Created by why on 3/23/24.
//

#include "morphine/gc/finalizer.h"
#include "morphine/core/call.h"
#include "morphine/core/instance.h"
#include "morphine/object/native.h"

static void finalizer(morphine_state_t S) {
    morphine_instance_t I = S->I;

    if (callI_callstate(S) == 0) {
        callI_continue(S, 0);

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

        callI_continue(S, 1);

        I->G.finalizer.candidate = candidate;

        struct value candidate_value = valueI_object(candidate);
        struct value callable = valueI_nil;
        candidate->flags.finalized = true;
        if (metatableI_test(S->I, candidate_value, MF_GC, &callable)) {
            callI_do(S, callable, candidate_value, 0, NULL, 0);
            return;
        }
    }

    if (callI_callstate(S) == 1) {
        callI_continue(S, 0);

        struct object *candidate = I->G.finalizer.candidate;
        I->G.finalizer.candidate = NULL;

        if (candidate != NULL) {
            candidate->prev = I->G.pools.allocated;
            I->G.pools.allocated = candidate;
        }

        return;
    }

    throwI_message_panic(I, S, "Undefined gc finalizer's state");
}

void gcI_init_finalizer(morphine_instance_t I) {
    morphine_state_t state = stateI_custom_create(
        I,
        I->params.gc.finalizer.stack_limit,
        I->params.gc.finalizer.stack_grow
    );

    stateI_priority(state, 1);

    struct native *native = nativeI_create(I, "gc_finalizer", finalizer);
    callI_do(state, valueI_object(native), valueI_nil, 0, NULL, 0);

    I->state_finalizer = state;
}