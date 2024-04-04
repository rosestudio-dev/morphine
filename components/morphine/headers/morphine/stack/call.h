//
// Created by whyiskra on 3/24/24.
//

#pragma once

#include "morphine/core/throw.h"
#include "morphine/core/value.h"
#include "morphine/object/string.h"
#include "morphine/object/coroutine.h"

#define callstackI_info(U) (U)->stack.callstack
#define callstackI_info_or_error(U) ({ morphine_coroutine_t s = (U); struct callinfo *c = callstackI_info(s); if(unlikely(c == NULL)) throwI_errorf(s->I, "Require callable"); c; })

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
    morphine_coroutine_t U,
    struct value callable,
    struct value self,
    struct value *args,
    size_t argc,
    size_t pop_size
);

void callstackI_stack(
    morphine_coroutine_t U,
    struct value callable,
    struct value self,
    size_t offset,
    size_t argc,
    size_t pop_size
);

void callstackI_params(
    morphine_coroutine_t U,
    struct value callable,
    struct value self,
    size_t argc,
    size_t pop_size
);

void callstackI_pop(morphine_coroutine_t);

void callstackI_info_free(morphine_instance_t, struct callinfo *callinfo);
struct value callstackI_extract_callable(morphine_instance_t, struct value callable);

static inline struct value callstackI_result(morphine_coroutine_t U) {
    return *callstackI_info_or_error(U)->s.result.p;
}

static inline void callstackI_return(morphine_coroutine_t U, struct value value) {
    struct callinfo *callinfo = callstackI_info_or_error(U);

    if (callinfo->prev != NULL) {
        *callinfo->prev->s.result.p = value;
    }

    callinfo->exit = true;
}

static inline void callstackI_leave(morphine_coroutine_t U) {
    callstackI_return(U, valueI_nil);
}

static inline void callstackI_continue(morphine_coroutine_t U, size_t state) {
    struct callinfo *callinfo = callstackI_info_or_error(U);
    callinfo->pc.state = state;
    callinfo->exit = false;
}

static inline size_t callstackI_state(morphine_coroutine_t U) {
    return callstackI_info_or_error(U)->pc.state;
}
