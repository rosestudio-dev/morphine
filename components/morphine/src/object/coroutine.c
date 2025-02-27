//
// Created by whyiskra on 16.12.23.
//

#include <string.h>
#include "morphine/object/coroutine.h"
#include "morphine/object/string.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/safe.h"

static void attach(morphine_coroutine_t U) {
    if (U == NULL) {
        return;
    }

    morphine_instance_t I = U->I;

    morphine_coroutine_t current = I->interpreter.coroutines;
    while (current != NULL && current != U) {
        current = current->prev;
    }

    if (current != NULL) {
        return;
    }

    U->prev = I->interpreter.coroutines;
    I->interpreter.coroutines = U;
}

static void detach(morphine_coroutine_t U) {
    if (U == NULL) {
        return;
    }

    morphine_instance_t I = U->I;

    morphine_coroutine_t last = NULL;
    morphine_coroutine_t current = I->interpreter.coroutines;
    while (current != NULL && current != U) {
        last = current;
        current = current->prev;
    }

    if (current == NULL) {
        return;
    }

    if (last == NULL) {
        I->interpreter.coroutines = current->prev;
    } else {
        last->prev = current->prev;
    }

    if (current == I->interpreter.next) {
        I->interpreter.next = current->prev;
    }

    current->prev = NULL;
}

morphine_coroutine_t coroutineI_create(morphine_instance_t I, struct string *name, struct value env) {
    if (name == NULL) {
        throwI_error(I, "coroutine name is null");
    }

    gcI_safe_enter(I);
    gcI_safe(I, valueI_object(name));
    gcI_safe(I, env);

    // create
    morphine_coroutine_t result = allocI_uni(I, NULL, sizeof(struct coroutine));
    (*result) = (struct coroutine) {
        .name = name,
        .status = COROUTINE_STATUS_CREATED,
        .priority = 1,
        .stack = stackI_prototype(I),
        .callstack = callstackI_prototype(),
        .env = env,
        .result = valueI_nil,
        .thrown.type = THROW_TYPE_UNDEF,
        .thrown.exception = NULL,
        .prev = NULL,
        .I = I
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_COROUTINE);

    gcI_safe_exit(I);

    return result;
}

void coroutineI_free(morphine_instance_t I, morphine_coroutine_t coroutine) {
    callstackI_destruct(I, &coroutine->callstack);
    stackI_destruct(I, &coroutine->stack);
    allocI_free(I, coroutine);
}

void coroutineI_priority(morphine_coroutine_t U, ml_size priority) {
    if (priority <= 0) {
        U->priority = 1;
    } else {
        U->priority = priority;
    }
}

void coroutineI_attach(morphine_coroutine_t U) {
    switch (U->status) {
        case COROUTINE_STATUS_RUNNING:
        case COROUTINE_STATUS_SUSPENDED:
            throwI_error(U->I, "coroutine is already attached");
        case COROUTINE_STATUS_DEAD:
            throwI_error(U->I, "coroutine is dead");
        case COROUTINE_STATUS_CREATED: {
            attach(U);
            U->status = COROUTINE_STATUS_RUNNING;
            return;
        }
    }

    throwI_panic(U->I, "unknown coroutine status");
}

void coroutineI_suspend(morphine_coroutine_t U) {
    switch (U->status) {
        case COROUTINE_STATUS_RUNNING:
            U->status = COROUTINE_STATUS_SUSPENDED;
            return;
        case COROUTINE_STATUS_SUSPENDED:
            return;
        case COROUTINE_STATUS_DEAD:
            throwI_error(U->I, "coroutine is dead");
        case COROUTINE_STATUS_CREATED:
            throwI_error(U->I, "coroutine isn't running");
    }

    throwI_panic(U->I, "unknown coroutine status");
}

void coroutineI_resume(morphine_coroutine_t U) {
    switch (U->status) {
        case COROUTINE_STATUS_RUNNING:
            return;
        case COROUTINE_STATUS_SUSPENDED:
            U->status = COROUTINE_STATUS_RUNNING;
            return;
        case COROUTINE_STATUS_DEAD:
            throwI_error(U->I, "coroutine is dead");
        case COROUTINE_STATUS_CREATED:
            throwI_error(U->I, "coroutine isn't running");
    }

    throwI_panic(U->I, "unknown coroutine status");
}

void coroutineI_kill(morphine_coroutine_t U) {
    detach(U);
    U->status = COROUTINE_STATUS_DEAD;
}

bool coroutineI_isalive(morphine_coroutine_t U) {
    switch (U->status) {
        case COROUTINE_STATUS_CREATED:
        case COROUTINE_STATUS_RUNNING:
        case COROUTINE_STATUS_SUSPENDED:
            return true;
        case COROUTINE_STATUS_DEAD:
            return false;
    }

    throwI_panic(U->I, "unknown coroutine status");
}

const char *coroutineI_status2string(morphine_coroutine_t U, enum coroutine_status status) {
    switch (status) {
        case COROUTINE_STATUS_RUNNING:
            return "running";
        case COROUTINE_STATUS_SUSPENDED:
            return "suspended";
        case COROUTINE_STATUS_DEAD:
            return "dead";
        case COROUTINE_STATUS_CREATED:
            return "created";
    }

    throwI_panic(U->I, "unknown coroutine status");
}
