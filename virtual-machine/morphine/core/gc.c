//
// Created by whyiskra on 16.12.23.
//

#include "morphine/core/gc.h"
#include "morphine/object/state.h"
#include "morphine/object/table.h"
#include "morphine/object/closure.h"
#include "morphine/object/userdata.h"
#include "morphine/object/proto.h"
#include "morphine/object/reference.h"
#include "morphine/object/string.h"
#include "morphine/object/native.h"
#include "morphine/core/throw.h"
#include "morphine/core/hashmap.h"
#include "morphine/core/instance.h"
#include "morphine/core/call.h"
#include "morphine/core/hook.h"

struct garbage_collector gcI_init(struct params params, size_t inited_size) {
    return (struct garbage_collector) {
        .status = GC_STATUS_IDLE,
        .enabled = false,
        .cycle = 1,

        .settings.limit_bytes = params.gc.limit_bytes,
        .settings.start = params.gc.start,
        .settings.grow = params.gc.grow,
        .settings.deal = params.gc.deal,

        .bytes.started = inited_size,
        .bytes.allocated = inited_size,
        .bytes.max_allocated = inited_size,
        .bytes.prev_allocated = 0,

        .pools.allocated = NULL,
        .pools.gray = NULL,
        .pools.white = NULL,
        .pools.finalize_candidates = NULL,

        .finalize_candidate = NULL,

        .callinfo_trash = NULL,
        .finalize = false
    };
}

void gcI_enable(morphine_instance_t I) {
    I->G.enabled = true;
}

void gcI_disable(morphine_instance_t I) {
    I->G.enabled = false;
}

bool gcI_isenabled(morphine_instance_t I) {
    return I->G.enabled;
}

const char *gcI_status(morphine_instance_t I) {
    switch (I->G.status) {
        case GC_STATUS_IDLE:
            return "idle";
        case GC_STATUS_PREPARE:
            return "prepare";
        case GC_STATUS_INCREMENT:
            return "increment";
        case GC_STATUS_FINALIZE_PREPARE:
            return "finalize_prepare";
        case GC_STATUS_FINALIZE_INCREMENT:
            return "finalize_increment";
        case GC_STATUS_SWEEP:
            return "sweep";
    }

    throwI_message_panic(I, NULL, "Unsupported gc status");
}

void gcI_force(morphine_instance_t I) {
    if (I->G.status == GC_STATUS_IDLE) {
        I->G.status = GC_STATUS_PREPARE;
    }
}

bool gcI_isrunning(morphine_instance_t I) {
    return I->G.status != GC_STATUS_IDLE;
}

void gcI_recognize_start(morphine_instance_t I) {
    I->G.bytes.started = I->G.bytes.allocated;
}

void gcI_dispose_callinfo(morphine_instance_t I, struct callinfo *callinfo) {
    callinfo->prev = I->G.callinfo_trash;
    I->G.callinfo_trash = callinfo;
}

struct callinfo *gcI_get_hot_callinfo(morphine_instance_t I) {
    struct callinfo *callinfo = I->G.callinfo_trash;

    if (callinfo != NULL) {
        I->G.callinfo_trash = callinfo->prev;
    }

    return callinfo;
}

static void finalizer(morphine_state_t S) {
    morphine_instance_t I = S->I;

    if (callI_callstate(S) == 0) {
        callI_continue(S, 0);

        struct object *candidate = I->G.pools.finalize_candidates;
        if (candidate == NULL) {
            I->G.finalize = false;
            return;
        }

        I->G.pools.finalize_candidates = candidate->prev;

        if (candidate->mark) {
            candidate->prev = I->G.pools.allocated;
            I->G.pools.allocated = candidate;
            return;
        }

        callI_continue(S, 1);

        I->G.finalize_candidate = candidate;

        struct value candidate_value = valueI_object(candidate);
        struct value callable = valueI_nil;
        if (metatableI_test(S->I, candidate_value, MF_GC, &callable)) {
            callI_do(S, callable, candidate_value, 0, NULL, 0);
            return;
        } else {
            return;
        }
    } else if (callI_callstate(S) == 1) {
        callI_continue(S, 0);

        if (I->G.finalize_candidate != NULL) {
            objectI_free(I, I->G.finalize_candidate);
        }

        I->G.finalize_candidate = NULL;
    } else {
        throwI_message_panic(I, S, "Undefined gc finalizer's state");
    }
}

void gcI_finalizer_state(morphine_instance_t I) {
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

static inline void ofm_check(morphine_instance_t I) {
    if (I->G.bytes.allocated >= I->G.settings.limit_bytes) {
        throwI_message_panic(I, NULL, "Out of memory");
    }
}

static inline void mark_object(struct object *object) {
    object->mark = true;
}

static inline void mark_value(struct value value) {
    if (valueI_is_object(value)) {
        mark_object(valueI_as_object(value));
    }
}

static inline size_t mark_internal(morphine_instance_t I, struct object *obj) {
    switch (obj->type) {
        case OBJ_TYPE_TABLE: {
            struct table *table = morphinem_cast(struct table *, obj);

            if (table->metatable != NULL) {
                mark_object(objectI_cast(table->metatable));
            }

            size_t it = 0;
            struct pair pair;

            while (hashmap_iter(table->hashmap, &it, &pair)) {
                mark_value(pair.key);
                mark_value(pair.value);
            }

            return tableI_allocated_size(table);
        }
        case OBJ_TYPE_CLOSURE: {
            struct closure *closure = morphinem_cast(struct closure *, obj);

            mark_value(closure->callable);
            for (size_t i = 0; i < closure->size; i++) {
                mark_value(closure->values[i]);
            }

            return closureI_allocated_size(closure);
        }
        case OBJ_TYPE_PROTO: {
            struct proto *proto = morphinem_cast(struct proto *, obj);

            for (size_t i = 0; i < proto->constants_count; i++) {
                mark_value(proto->constants[i]);
            }

            for (size_t i = 0; i < proto->statics_count; i++) {
                mark_value(proto->statics[i]);
            }

            mark_value(proto->registry_key);

            return protoI_allocated_size(proto);
        }
        case OBJ_TYPE_USERDATA: {
            struct userdata *userdata = morphinem_cast(struct userdata *, obj);

            if (userdata->metatable != NULL) {
                mark_object(objectI_cast(userdata->metatable));
            }

            if (userdata->mark != NULL) {
                userdata->mark(I, userdata->userdata);
            }

            return userdataI_allocated_size(userdata);
        }
        case OBJ_TYPE_STATE: {
            morphine_state_t state = morphinem_cast(morphine_state_t, obj);

            for (size_t i = 0; i < state->stack.top; i++) {
                mark_value(state->stack.allocated[i]);
            }

            return stateI_allocated_size(state);
        }
        case OBJ_TYPE_NATIVE: {
            struct native *native = morphinem_cast(struct native *, obj);
            mark_value(native->registry_key);

            return nativeI_allocated_size(native);
        }
        case OBJ_TYPE_STRING: {
            struct string *string = morphinem_cast(struct string *, obj);
            return stringI_allocated_size(string);
        }
        case OBJ_TYPE_REFERENCE: {
            struct reference *reference = morphinem_cast(struct reference *, obj);
            return referenceI_allocated_size(reference);
        }
    }

    throwI_message_panic(I, NULL, "Unsupported object");
}

static inline void record(morphine_instance_t I) {
    {
        morphine_state_t current_state = I->states;
        while (current_state != NULL) {
            mark_object(objectI_cast(current_state));
            current_state = current_state->prev;
        }

        if (I->state_finalizer != NULL) {
            mark_object(objectI_cast(I->state_finalizer));
        }
    }

    {
        morphine_state_t current_state = I->candidates;
        while (current_state != NULL) {
            mark_object(objectI_cast(current_state));
            current_state = current_state->prev;
        }
    }

    {
        struct object *current = I->G.pools.finalize_candidates;
        while (current != NULL) {
            mark_internal(I, current);
            current = current->prev;
        }

        if (I->G.finalize_candidate != NULL) {
            mark_internal(I, I->G.finalize_candidate);
        }
    }

    for (enum metatable_field mf = MFS_START; mf < MFS_COUNT; mf++) {
        mark_object(objectI_cast(I->metatable.names[mf]));
    }

    for (enum value_type type = VALUE_TYPES_START; type < VALUE_TYPES_COUNT; type++) {
        struct table *table = I->metatable.defaults[type];
        if (table != NULL) {
            mark_object(objectI_cast(table));
        }
    }

    if (!I->throw.is_message) {
        mark_value(I->throw.result.value);
    }

    if (I->env != NULL) {
        mark_object(objectI_cast(I->env));
    }

    if (I->registry != NULL) {
        mark_object(objectI_cast(I->registry));
    }
}

static inline void prepare(morphine_instance_t I) {
    if (morphinem_unlikely(I->G.pools.gray != NULL || I->G.pools.white != NULL)) {
        throwI_message_panic(I, NULL, "Corrupted gc pools");
    }

    I->G.pools.gray = NULL;
    I->G.pools.white = NULL;

    {
        struct object *current = I->G.pools.allocated;
        while (current != NULL) {
            current->mark = false;
            current = current->prev;
        }
    }

    record(I);
}

static inline void movegray(morphine_instance_t I) {
    struct object *current = I->G.pools.allocated;
    struct object *pool = NULL;
    while (current != NULL) {
        struct object *prev = current->prev;

        if (current->mark) {
            current->prev = I->G.pools.gray;
            I->G.pools.gray = current;
        } else {
            current->prev = pool;
            pool = current;
        }

        current = prev;
    }

    I->G.pools.allocated = pool;
}

static inline void checks(morphine_instance_t I) {
    {
        morphine_state_t current_state = I->states;
        while (current_state != NULL) {
            mark_object(objectI_cast(current_state));
            mark_internal(I, objectI_cast(current_state));
            current_state = current_state->prev;
        }
    }

    {
        morphine_state_t current_state = I->candidates;
        while (current_state != NULL) {
            mark_object(objectI_cast(current_state));
            mark_internal(I, objectI_cast(current_state));
            current_state = current_state->prev;
        }
    }

    for (enum metatable_field mf = MFS_START; mf < MFS_COUNT; mf++) {
        mark_object(objectI_cast(I->metatable.names[mf]));
    }

    for (enum value_type type = VALUE_TYPES_START; type < VALUE_TYPES_COUNT; type++) {
        struct table *table = I->metatable.defaults[type];
        if (table != NULL) {
            mark_object(objectI_cast(table));
        }
    }

    if (!I->throw.is_message) {
        mark_value(I->throw.result.value);
    }

    if (I->env != NULL) {
        mark_object(objectI_cast(I->env));
    }

    if (I->registry != NULL) {
        mark_object(objectI_cast(I->registry));
    }
}

static inline bool increment(morphine_instance_t I) {
    size_t deal = 0;
    size_t expected_deal = (I->G.bytes.allocated * I->G.settings.deal) / 100;

retry:
    movegray(I);

    struct object *pool = I->G.pools.gray;

    {
        struct object *current = pool;
        while (current != NULL) {
            struct object *prev = current->prev;

            current->prev = I->G.pools.white;
            I->G.pools.white = current;

            deal += mark_internal(I, current);

            current = prev;
        }

        I->G.pools.gray = NULL;
    }

    if (morphinem_unlikely(pool == NULL)) {
        checks(I);
        movegray(I);

        pool = I->G.pools.gray;
    }

    if (morphinem_unlikely(pool == NULL)) {
        return true;
    }

    if (expected_deal > deal) {
        goto retry;
    }

    return false;
}

static inline void resolve_refs(morphine_instance_t I) {
    struct object *current = I->G.pools.white;
    while (current != NULL) {
        if (current->type == OBJ_TYPE_REFERENCE) {
            referenceI_invalidate(morphinem_cast(struct reference *, current));
        }

        current = current->prev;
    }
}

static inline void shrink(morphine_instance_t I) {
    {
        morphine_state_t current = I->states;
        while (current != NULL) {
            stackI_shrink(current);
            current = current->prev;
        }
    }

    {
        struct callinfo *current = I->G.callinfo_trash;
        while (current != NULL) {
            struct callinfo *prev = current->prev;
            stackI_callinfo_free(I, current);

            current = prev;
        }

        I->G.callinfo_trash = NULL;
    }
}

static inline bool finalize(morphine_instance_t I) {
    struct object *current = I->G.pools.allocated;
    struct object *pool = NULL;

    bool has = false;
    while (current != NULL) {
        struct object *prev = current->prev;

        if (morphinem_unlikely(metatableI_test(I, valueI_object(current), MF_GC, NULL))) {
            current->prev = I->G.pools.finalize_candidates;
            I->G.pools.finalize_candidates = current;

            mark_internal(I, current);
            has = true;
        } else {
            current->prev = pool;
            pool = current;
        }

        current = prev;
    }

    I->G.pools.allocated = pool;

    if (has) {
        I->G.finalize = true;
    }

    return has;
}

static inline void sweep(morphine_instance_t I) {
    struct garbage_collector *G = &I->G;

    resolve_refs(I);

    struct object *current = G->pools.allocated;
    while (current != NULL) {
        struct object *prev = current->prev;

        objectI_free(I, current);

        current = prev;
    }

    G->pools.allocated = G->pools.white;
    G->pools.white = NULL;

    shrink(I);

    size_t start = G->bytes.started * I->G.settings.start / 100;

    if (G->bytes.allocated < start) {
        G->bytes.prev_allocated = start;
    } else {
        G->bytes.prev_allocated = G->bytes.allocated;
    }

    G->cycle++;

    ofm_check(I);
}

static inline void step(morphine_instance_t I) {
    pdbg_hook_gc_step_enter(I);

    if (I->G.bytes.allocated >= I->G.settings.limit_bytes) {
        pdbg_hook_gc_step_exit(I);
        gcI_full(I);
        return;
    }

    while (true) {
        switch (I->G.status) {
            case GC_STATUS_PREPARE: {
                prepare(I);
                I->G.status = GC_STATUS_INCREMENT;
                goto exit;
            }
            case GC_STATUS_INCREMENT: {
                if (increment(I)) {
                    I->G.status = GC_STATUS_FINALIZE_PREPARE;
                    break;
                } else {
                    goto exit;
                }
            }
            case GC_STATUS_FINALIZE_PREPARE: {
                if (finalize(I)) {
                    I->G.status = GC_STATUS_FINALIZE_INCREMENT;
                    goto exit;
                } else {
                    I->G.status = GC_STATUS_SWEEP;
                    break;
                }
            }
            case GC_STATUS_FINALIZE_INCREMENT: {
                if (increment(I)) {
                    I->G.status = GC_STATUS_SWEEP;
                    break;
                } else {
                    goto exit;
                }
            }
            case GC_STATUS_SWEEP: {
                sweep(I);
                I->G.status = GC_STATUS_IDLE;
                goto exit;
            }
            case GC_STATUS_IDLE: {
                goto exit;
            }
        }

        pdbg_hook_gc_step_middle(I);
    }

exit:
    pdbg_hook_gc_step_exit(I);
}

static inline bool gc_need(morphine_instance_t I) {
    size_t alloc_bytes = I->G.bytes.allocated;
    size_t prev = I->G.bytes.prev_allocated;

    if (morphinem_unlikely(prev == 0)) {
        prev = 1;
    }

    size_t percent = (100 * alloc_bytes + prev / 2) / prev;

    bool start = percent >= I->G.settings.grow;

    return start || (alloc_bytes >= I->G.settings.limit_bytes);
}

void gcI_work(morphine_instance_t I) {
    if (!I->G.enabled) {
        ofm_check(I);
        return;
    }

    if (I->G.status != GC_STATUS_IDLE) {
        step(I);
        return;
    }

    if (gc_need(I)) {
        I->G.status = GC_STATUS_PREPARE;
        step(I);
    }
}

void gcI_full(morphine_instance_t I) {
    pdbg_hook_gc_full_enter(I);

    if (I->G.pools.white != NULL) {
        I->G.pools.white->prev = I->G.pools.allocated;
        I->G.pools.allocated = I->G.pools.white;
    }

    if (I->G.pools.gray != NULL) {
        I->G.pools.gray->prev = I->G.pools.allocated;
        I->G.pools.allocated = I->G.pools.gray;
    }

    prepare(I);
    while (!increment(I)) { }

    if (finalize(I)) {
        while (!increment(I)) { }
    }

    sweep(I);

    I->G.status = GC_STATUS_IDLE;

    pdbg_hook_gc_full_exit(I);
}
