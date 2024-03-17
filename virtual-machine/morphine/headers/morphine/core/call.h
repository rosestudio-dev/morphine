//
// Created by whyiskra on 23.12.23.
//

#pragma once

#include "morphine/object/state.h"
#include "throw.h"

void callI_do(
    morphine_state_t,
    struct value callable,
    struct value self,
    size_t argc,
    struct value *args,
    size_t pop_size
);

static inline struct value callI_result(morphine_state_t S) {
    return *stackI_callinfo_or_error(S)->s.result.p;
}

static inline void callI_return(morphine_state_t S, struct value value) {
    struct callinfo *callinfo = stackI_callinfo_or_error(S);

    if (callinfo->prev != NULL) {
        *callinfo->prev->s.result.p = value;
    }

    callinfo->exit = true;
}

static inline void callI_leave(morphine_state_t S) {
    callI_return(S, valueI_nil);
}

static inline void callI_continue(morphine_state_t S, size_t callstate) {
    struct callinfo *callinfo = stackI_callinfo_or_error(S);
    callinfo->pc.callstate = callstate;
    callinfo->exit = false;
}

static inline size_t callI_callstate(morphine_state_t S) {
    return stackI_callinfo_or_error(S)->pc.callstate;
}
