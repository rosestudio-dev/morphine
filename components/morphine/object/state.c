//
// Created by whyiskra on 16.12.23.
//

#include <string.h>
#include "morphine/object/state.h"
#include "morphine/object/string.h"
#include "morphine/core/instance.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"

morphine_state_t stateI_custom_create(morphine_instance_t I, size_t stack_limit, size_t stack_grow) {
    morphine_state_t result = allocI_uni(I, NULL, sizeof(struct state));

    (*result) = (struct state) {
        .status = STATE_STATUS_CREATED,
        .priority = 1,
        .prev = NULL,
        .I = I
    };

    result->stack = stackI_initial(I, stack_limit, stack_grow);

    objectI_init(I, objectI_cast(result), OBJ_TYPE_STATE);

    return result;
}

morphine_state_t stateI_create(morphine_instance_t I) {
    morphine_state_t result = stateI_custom_create(
        I,
        I->settings.states.stack_limit,
        I->settings.states.stack_grow
    );

    return result;
}

void stateI_free(morphine_instance_t I, morphine_state_t state) {
    stackI_destruct(I, state->stack);
    allocI_free(I, state);
}

void stateI_priority(morphine_state_t S, priority_t priority) {
    if (priority <= 0) {
        S->priority = 1;
    } else {
        S->priority = priority;
    }
}

void stateI_attach(morphine_state_t S) {
    switch (S->status) {
        case STATE_STATUS_ATTACHED:
        case STATE_STATUS_RUNNING:
        case STATE_STATUS_SUSPENDED:
        case STATE_STATUS_DEAD:
        case STATE_STATUS_DETACHED:
            throwI_error(S->I, "State is already attached");
        case STATE_STATUS_CREATED: {
            morphine_instance_t I = S->I;
            S->prev = I->E.candidates;
            I->E.candidates = S;
            S->status = STATE_STATUS_ATTACHED;
            break;
        }
    }
}

void stateI_suspend(morphine_state_t S) {
    switch (S->status) {
        case STATE_STATUS_ATTACHED:
        case STATE_STATUS_RUNNING:
            S->status = STATE_STATUS_SUSPENDED;
            break;
        case STATE_STATUS_SUSPENDED:
            throwI_error(S->I, "State is suspended");
        case STATE_STATUS_DEAD:
            throwI_error(S->I, "State is dead");
        case STATE_STATUS_DETACHED:
            throwI_error(S->I, "State is detached");
        case STATE_STATUS_CREATED:
            throwI_error(S->I, "State isn't attached");
    }
}

void stateI_resume(morphine_state_t S) {
    switch (S->status) {
        case STATE_STATUS_ATTACHED:
        case STATE_STATUS_RUNNING:
            throwI_error(S->I, "State is running");
        case STATE_STATUS_SUSPENDED:
            S->status = STATE_STATUS_RUNNING;
            break;
        case STATE_STATUS_DEAD:
            throwI_error(S->I, "State is dead");
        case STATE_STATUS_DETACHED:
            throwI_error(S->I, "State is detached");
        case STATE_STATUS_CREATED:
            throwI_error(S->I, "State isn't attached");
    }
}

void stateI_kill(morphine_state_t S) {
    switch (S->status) {
        case STATE_STATUS_ATTACHED:
        case STATE_STATUS_RUNNING:
        case STATE_STATUS_SUSPENDED:
            S->status = STATE_STATUS_DEAD;
            break;
        case STATE_STATUS_DEAD:
            throwI_error(S->I, "State is dead");
        case STATE_STATUS_DETACHED:
            throwI_error(S->I, "State is detached");
        case STATE_STATUS_CREATED:
            throwI_error(S->I, "State isn't attached");
    }
}

void stateI_kill_regardless(morphine_state_t S) {
    switch (S->status) {
        case STATE_STATUS_CREATED:
        case STATE_STATUS_ATTACHED:
        case STATE_STATUS_RUNNING:
        case STATE_STATUS_SUSPENDED:
            S->status = STATE_STATUS_DEAD;
            break;
        case STATE_STATUS_DEAD:
        case STATE_STATUS_DETACHED:
            break;
    }
}

bool stateI_isalive(morphine_state_t S) {
    switch (S->status) {
        case STATE_STATUS_CREATED:
        case STATE_STATUS_ATTACHED:
        case STATE_STATUS_RUNNING:
        case STATE_STATUS_SUSPENDED:
            return true;
        case STATE_STATUS_DEAD:
        case STATE_STATUS_DETACHED:
            return false;
    }

    throwI_panic(S->I, "Unknown status");
}

const char *stateI_status2string(morphine_state_t S, enum state_status status) {
    switch (status) {
        case STATE_STATUS_ATTACHED:
            return "attached";
        case STATE_STATUS_RUNNING:
            return "running";
        case STATE_STATUS_SUSPENDED:
            return "suspended";
        case STATE_STATUS_DEAD:
            return "dead";
        case STATE_STATUS_DETACHED:
            return "detached";
        case STATE_STATUS_CREATED:
            return "created";
    }

    throwI_panic(S->I, "Unknown status");
}

enum state_status stateI_string2status(morphine_state_t S, const char *name) {
    for (enum state_status t = 0; t < STATE_STATUS_COUNT; t++) {
        if (strcmp(stateI_status2string(S, t), name) == 0) {
            return t;
        }
    }

    throwI_error(S->I, "Unknown status");
}
