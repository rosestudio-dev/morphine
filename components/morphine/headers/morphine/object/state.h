//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/stack/structure.h"
#include "morphine/core/object.h"

typedef enum {
    STATE_STATUS_ATTACHED,
    STATE_STATUS_RUNNING,
    STATE_STATUS_SUSPENDED,
    STATE_STATUS_DEAD,
    STATE_STATUS_DETACHED,
} state_status_t;

#define STATE_STATUS_COUNT (STATE_STATUS_DEAD + 1)

struct state {
    struct object header;

    state_status_t status;
    priority_t priority;

    struct stack stack;

    morphine_state_t prev;
    morphine_instance_t I;
};

morphine_state_t stateI_custom_create(morphine_instance_t, size_t stack_limit, size_t stack_grow);
morphine_state_t stateI_create(morphine_instance_t);
void stateI_free(morphine_instance_t, morphine_state_t state);

size_t stateI_allocated_size(morphine_state_t);

void stateI_priority(morphine_state_t, priority_t);
void stateI_attach(morphine_state_t);
void stateI_suspend(morphine_state_t);
void stateI_resume(morphine_state_t);
void stateI_kill(morphine_state_t);
void stateI_kill_regardless(morphine_state_t);
bool stateI_isalive(morphine_state_t S);

const char *stateI_status2string(morphine_state_t, state_status_t status);
state_status_t stateI_string2status(morphine_state_t, const char *name);
