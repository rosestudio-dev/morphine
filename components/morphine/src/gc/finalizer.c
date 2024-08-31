//
// Created by whyiskra on 3/23/24.
//

#include "morphine/gc/finalizer.h"
#include "morphine/core/instance.h"
#include "morphine/object/native.h"
#include "morphine/object/coroutine.h"
#include "morphine/gc/pools.h"

#define COROUTINE_FINALIZER_NAME "__gc_finalizer"

static void give_away(morphine_instance_t I, struct object *candidate) {
    if (I->G.status == GC_STATUS_INCREMENT) {
        candidate->color = OBJ_COLOR_GREY;
        gcI_pools_insert(candidate, &I->G.pools.grey);
    } else {
        candidate->color = OBJ_COLOR_WHITE;
        gcI_pools_insert(candidate, &I->G.pools.allocated);
    }
}

static void finalizer(morphine_coroutine_t U) {
    morphine_instance_t I = U->I;

    if (callstackI_state(U) == 0) {
        callstackI_continue(U, 0);

        struct object *candidate = I->G.pools.finalize;
        if (candidate == NULL) {
            I->G.finalizer.work = false;
            return;
        }

        gcI_pools_remove(candidate, &I->G.pools.finalize);

        if (candidate->flags.finalized) {
            give_away(I, candidate);
            return;
        }

        callstackI_continue(U, 1);

        I->G.finalizer.candidate = candidate;

        struct value candidate_value = valueI_object(candidate);
        struct value callable = valueI_nil;
        candidate->flags.finalized = true;
        if (metatableI_test(U->I, candidate_value, MF_GC, &callable)) {
            callstackI_call_unsafe(U, callable, candidate_value, NULL, 0, 0);
            return;
        }
    }

    if (callstackI_state(U) == 1) {
        callstackI_continue(U, 0);

        struct object *candidate = I->G.finalizer.candidate;
        I->G.finalizer.candidate = NULL;

        if (candidate != NULL) {
            give_away(I, candidate);
        }

        return;
    }

    throwI_panic(I, "undefined gc finalizer's state");
}

void gcI_init_finalizer(morphine_instance_t I) {
    morphine_coroutine_t coroutine = coroutineI_custom_create(
        I, COROUTINE_FINALIZER_NAME,
        valueI_nil,
        I->settings.finalizer.stack_limit,
        I->settings.finalizer.stack_grow
    );

    coroutineI_priority(coroutine, 1);

    struct native *native = nativeI_create(I, "gc_finalizer", finalizer);
    callstackI_call_unsafe(coroutine, valueI_object(native), valueI_nil, NULL, 0, 0);

    I->G.finalizer.coroutine = coroutine;
}