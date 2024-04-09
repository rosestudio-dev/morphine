//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/object/coroutine/stack.h"
#include "morphine/core/value.h"

enum coroutine_status {
    COROUTINE_STATUS_CREATED,
    COROUTINE_STATUS_ATTACHED,
    COROUTINE_STATUS_RUNNING,
    COROUTINE_STATUS_SUSPENDED,
    COROUTINE_STATUS_DEAD,
    COROUTINE_STATUS_DETACHED,
};

#define COROUTINE_STATUS_COUNT (COROUTINE_STATUS_DETACHED + 1)

struct coroutine {
    struct object header;

    enum coroutine_status status;
    priority_t priority;

    struct stack stack;
    struct callstack callstack;
    struct value env;

    morphine_coroutine_t prev;
    morphine_instance_t I;
};

morphine_coroutine_t coroutineI_custom_create(
    morphine_instance_t,
    struct value env,
    size_t stack_limit,
    size_t stack_grow
);

morphine_coroutine_t coroutineI_create(morphine_instance_t, struct value env);
void coroutineI_free(morphine_instance_t, morphine_coroutine_t coroutine);

void coroutineI_priority(morphine_coroutine_t, priority_t);
void coroutineI_attach(morphine_coroutine_t);
void coroutineI_suspend(morphine_coroutine_t);
void coroutineI_resume(morphine_coroutine_t);
void coroutineI_kill(morphine_coroutine_t);
void coroutineI_kill_regardless(morphine_coroutine_t);
bool coroutineI_isalive(morphine_coroutine_t U);

const char *coroutineI_status2string(morphine_coroutine_t, enum coroutine_status status);
enum coroutine_status coroutineI_string2status(morphine_coroutine_t, const char *name);
