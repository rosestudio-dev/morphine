//
// Created by whyiskra on 3/23/24.
//

#include "morphine/gc/finalizer.h"
#include "morphine/core/instance.h"
#include "morphine/gc/pools.h"
#include "morphine/gc/safe.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/native.h"

#define FINALIZER_COROUTINE_NAME "gcfinalizer"
#define FINALIZER_NATIVE_NAME    "gcfinalizerresolver"

static void give_away(morphine_instance_t I, struct object *candidate) {
    if (I->G.status == GC_STATUS_INCREMENT) {
        candidate->color = OBJ_COLOR_GREY;
        gcI_pools_insert(candidate, &I->G.pools.grey);
    } else {
        candidate->color = OBJ_COLOR_WHITE;
        gcI_pools_insert(candidate, &I->G.pools.allocated);
    }
}

static void resolver(morphine_coroutine_t U) {
    morphine_instance_t I = U->I;

    callstackI_catchable(U, 1);
    switch (callstackI_state(U)) {
        case 0: {
            callstackI_continue(U, 0);

            struct object *candidate = I->G.pools.finalize;
            if (candidate == NULL) {
                goto leave;
            }

            gcI_pools_remove(candidate, &I->G.pools.finalize);

            if (candidate->flags.finalized) {
                give_away(I, candidate);
                goto leave;
            }

            I->G.finalizer.candidate = candidate;
            candidate->flags.finalized = true;

            callstackI_continue(U, 1);

            struct value candidate_value = valueI_object(candidate);
            struct value callable = valueI_nil;
            if (metatableI_builtin_test(U->I, candidate_value, MTYPE_METAFIELD_GC, &callable)) {
                callstackI_call(U, &callable, &candidate_value, 1, 0);
                return;
            }

            goto give;
        }
        case 1: {
give:;
            struct object *candidate = I->G.finalizer.candidate;
            I->G.finalizer.candidate = NULL;

            if (candidate != NULL) {
                give_away(I, candidate);
            }

            goto leave;
        }
        default: break;
    }

    throwI_panic(I, "undefined gc finalizer's state");

leave:
    callstackI_pop(U, valueI_nil);
}

void gcI_init_finalizer(morphine_instance_t I) {
    struct string *name = stringI_create(I, FINALIZER_NATIVE_NAME);
    I->G.finalizer.resolver = nativeI_create(I, name, resolver);
    I->G.finalizer.name = stringI_create(I, FINALIZER_COROUTINE_NAME);
}

void gcI_finalize(morphine_instance_t I) {
    if (I->G.finalizer.coroutine != NULL) {
        if (I->G.finalizer.coroutine->status == COROUTINE_STATUS_DETACHED) {
            I->G.finalizer.coroutine = NULL;
        }

        return;
    }

    if (I->G.pools.finalize != NULL && I->G.finalizer.candidate == NULL) {
        gcI_safe_enter(I);
        morphine_coroutine_t coroutine =
            gcI_safe_obj(I, coroutine, coroutineI_create(I, I->G.finalizer.name, valueI_nil));

        struct value resolver = valueI_object(I->G.finalizer.resolver);
        callstackI_call(coroutine, &resolver, NULL, 0, 0);

        I->G.finalizer.coroutine = coroutine;

        gcI_safe_exit(I);
    }
}
