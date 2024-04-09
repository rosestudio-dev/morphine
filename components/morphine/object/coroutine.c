//
// Created by whyiskra on 16.12.23.
//

#include <string.h>
#include "morphine/object/coroutine.h"
#include "morphine/object/string.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"

morphine_coroutine_t coroutineI_custom_create(
    morphine_instance_t I,
    struct value env,
    size_t stack_limit,
    size_t stack_grow
) {
    morphine_coroutine_t result = allocI_uni(I, NULL, sizeof(struct coroutine));

    (*result) = (struct coroutine) {
        .status = COROUTINE_STATUS_CREATED,
        .priority = 1,
        .stack = stackI_prototype(I, stack_limit, stack_grow),
        .callstack = callstackI_prototype(),
        .env = env,
        .prev = NULL,
        .I = I
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_COROUTINE);

    return result;
}

morphine_coroutine_t coroutineI_create(morphine_instance_t I, struct value env) {
    morphine_coroutine_t result = coroutineI_custom_create(
        I,
        env,
        I->settings.states.stack_limit,
        I->settings.states.stack_grow
    );

    return result;
}

void coroutineI_free(morphine_instance_t I, morphine_coroutine_t coroutine) {
    callstackI_destruct(I, &coroutine->callstack);
    stackI_destruct(I, &coroutine->stack);
    allocI_free(I, coroutine);
}

void coroutineI_priority(morphine_coroutine_t U, priority_t priority) {
    if (priority <= 0) {
        U->priority = 1;
    } else {
        U->priority = priority;
    }
}

void coroutineI_attach(morphine_coroutine_t U) {
    switch (U->status) {
        case COROUTINE_STATUS_ATTACHED:
        case COROUTINE_STATUS_RUNNING:
        case COROUTINE_STATUS_SUSPENDED:
            throwI_error(U->I, "Coroutine is already attached");
        case COROUTINE_STATUS_DEAD:
        case COROUTINE_STATUS_DETACHED:
            throwI_error(U->I, "Coroutine is dead");
        case COROUTINE_STATUS_CREATED: {
            morphine_instance_t I = U->I;
            U->prev = I->E.candidates;
            I->E.candidates = U;
            U->status = COROUTINE_STATUS_ATTACHED;
            break;
        }
    }
}

void coroutineI_suspend(morphine_coroutine_t U) {
    switch (U->status) {
        case COROUTINE_STATUS_ATTACHED:
        case COROUTINE_STATUS_RUNNING:
            U->status = COROUTINE_STATUS_SUSPENDED;
            break;
        case COROUTINE_STATUS_SUSPENDED:
            throwI_error(U->I, "Coroutine is suspended");
        case COROUTINE_STATUS_DEAD:
            throwI_error(U->I, "Coroutine is dead");
        case COROUTINE_STATUS_DETACHED:
            throwI_error(U->I, "Coroutine is detached");
        case COROUTINE_STATUS_CREATED:
            throwI_error(U->I, "Coroutine isn't attached");
    }
}

void coroutineI_resume(morphine_coroutine_t U) {
    switch (U->status) {
        case COROUTINE_STATUS_ATTACHED:
        case COROUTINE_STATUS_RUNNING:
            throwI_error(U->I, "Coroutine is running");
        case COROUTINE_STATUS_SUSPENDED:
            U->status = COROUTINE_STATUS_RUNNING;
            break;
        case COROUTINE_STATUS_DEAD:
            throwI_error(U->I, "Coroutine is dead");
        case COROUTINE_STATUS_DETACHED:
            throwI_error(U->I, "Coroutine is detached");
        case COROUTINE_STATUS_CREATED:
            throwI_error(U->I, "Coroutine isn't attached");
    }
}

void coroutineI_kill(morphine_coroutine_t U) {
    switch (U->status) {
        case COROUTINE_STATUS_ATTACHED:
        case COROUTINE_STATUS_RUNNING:
        case COROUTINE_STATUS_SUSPENDED:
            U->status = COROUTINE_STATUS_DEAD;
            break;
        case COROUTINE_STATUS_DEAD:
            throwI_error(U->I, "Coroutine is dead");
        case COROUTINE_STATUS_DETACHED:
            throwI_error(U->I, "Coroutine is detached");
        case COROUTINE_STATUS_CREATED:
            throwI_error(U->I, "Coroutine isn't attached");
    }
}

void coroutineI_kill_regardless(morphine_coroutine_t U) {
    switch (U->status) {
        case COROUTINE_STATUS_CREATED:
        case COROUTINE_STATUS_ATTACHED:
        case COROUTINE_STATUS_RUNNING:
        case COROUTINE_STATUS_SUSPENDED:
            U->status = COROUTINE_STATUS_DEAD;
            break;
        case COROUTINE_STATUS_DEAD:
        case COROUTINE_STATUS_DETACHED:
            break;
    }
}

bool coroutineI_isalive(morphine_coroutine_t U) {
    switch (U->status) {
        case COROUTINE_STATUS_CREATED:
        case COROUTINE_STATUS_ATTACHED:
        case COROUTINE_STATUS_RUNNING:
        case COROUTINE_STATUS_SUSPENDED:
            return true;
        case COROUTINE_STATUS_DEAD:
        case COROUTINE_STATUS_DETACHED:
            return false;
    }

    throwI_panic(U->I, "Unknown status");
}

const char *coroutineI_status2string(morphine_coroutine_t U, enum coroutine_status status) {
    switch (status) {
        case COROUTINE_STATUS_ATTACHED:
            return "attached";
        case COROUTINE_STATUS_RUNNING:
            return "running";
        case COROUTINE_STATUS_SUSPENDED:
            return "suspended";
        case COROUTINE_STATUS_DEAD:
            return "dead";
        case COROUTINE_STATUS_DETACHED:
            return "detached";
        case COROUTINE_STATUS_CREATED:
            return "created";
    }

    throwI_panic(U->I, "Unknown status");
}

enum coroutine_status coroutineI_string2status(morphine_coroutine_t U, const char *name) {
    for (enum coroutine_status t = 0; t < COROUTINE_STATUS_COUNT; t++) {
        if (strcmp(coroutineI_status2string(U, t), name) == 0) {
            return t;
        }
    }

    throwI_error(U->I, "Unknown status");
}
