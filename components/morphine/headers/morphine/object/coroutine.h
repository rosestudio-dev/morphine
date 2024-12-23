//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/stack.h"
#include "morphine/core/value.h"

enum coroutine_status {
    COROUTINE_STATUS_CREATED,
    COROUTINE_STATUS_RUNNING,
    COROUTINE_STATUS_SUSPENDED,
    COROUTINE_STATUS_DEAD,
};

struct coroutine {
    struct object header;
    struct string *name;

    enum coroutine_status status;
    ml_size priority;

    struct stack stack;
    struct callstack callstack;
    struct value env;
    struct value result;
    struct value thrown;

    morphine_coroutine_t prev;
    morphine_instance_t I;
};

morphine_coroutine_t coroutineI_create(morphine_instance_t, struct string *name, struct value env);
void coroutineI_free(morphine_instance_t, morphine_coroutine_t coroutine);

void coroutineI_priority(morphine_coroutine_t, ml_size priority);
void coroutineI_attach(morphine_coroutine_t);
void coroutineI_suspend(morphine_coroutine_t);
void coroutineI_resume(morphine_coroutine_t);
void coroutineI_kill(morphine_coroutine_t);
bool coroutineI_isalive(morphine_coroutine_t U);

const char *coroutineI_status2string(morphine_coroutine_t, enum coroutine_status status);
