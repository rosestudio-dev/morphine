//
// Created by whyiskra on 3/24/24.
//

#pragma once

#include "morphine/core/value.h"
#include "morphine/core/throw.h"
#include "morphine/object/state.h"

#define callstackI_info(S) (S)->stack.callstack
#define callstackI_info_or_error(S) ({ morphine_state_t s = (S); struct callinfo *c = callstackI_info(s); if(morphinem_unlikely(c == NULL)) throwI_message_error(s, "Require callable"); c; })

typedef union {
    struct value *p;
    size_t diff;
} stackI_ptr;

struct callinfo {
    struct {
        union {
            stackI_ptr base;
            stackI_ptr callable;
        };

        stackI_ptr source;
        stackI_ptr env;
        stackI_ptr self;
        stackI_ptr result;
        stackI_ptr thrown;
        stackI_ptr args;
        stackI_ptr slots;
        stackI_ptr params;
        stackI_ptr space;
        stackI_ptr top;
    } s;

    size_t pop_size;

    size_t arguments_count;

    struct {
        size_t position;
        size_t state;
    } pc;

    struct {
        bool enable;
        size_t state;
        size_t space_size;
    } catch;

    bool exit;

    struct callinfo *prev;
};

void callstackI_unsafe(
    morphine_state_t S,
    struct value callable,
    struct value self,
    struct value *args,
    size_t argc,
    size_t pop_size
);

void callstackI_stack(
    morphine_state_t S,
    struct value callable,
    struct value self,
    size_t offset,
    size_t argc,
    size_t pop_size
);

void callstackI_params(
    morphine_state_t S,
    struct value callable,
    struct value self,
    size_t argc,
    size_t pop_size
);

void callstackI_pop(morphine_state_t);

void callstackI_info_free(morphine_instance_t, struct callinfo *callinfo);
struct value callstackI_extract_callable(morphine_instance_t, struct value callable);

static inline struct value callstackI_result(morphine_state_t S) {
    return *callstackI_info_or_error(S)->s.result.p;
}

static inline void callstackI_return(morphine_state_t S, struct value value) {
    struct callinfo *callinfo = callstackI_info_or_error(S);

    if (callinfo->prev != NULL) {
        *callinfo->prev->s.result.p = value;
    }

    callinfo->exit = true;
}

static inline void callstackI_leave(morphine_state_t S) {
    callstackI_return(S, valueI_nil);
}

static inline void callstackI_continue(morphine_state_t S, size_t state) {
    struct callinfo *callinfo = callstackI_info_or_error(S);
    callinfo->pc.state = state;
    callinfo->exit = false;
}

static inline size_t callstackI_state(morphine_state_t S) {
    return callstackI_info_or_error(S)->pc.state;
}
