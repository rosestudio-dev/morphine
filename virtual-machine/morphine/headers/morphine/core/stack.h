//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "value.h"

#define stackI_callinfo(S) (S)->stack.callstack
#define stackI_callinfo_or_error(S) ({ morphine_state_t s = (S); struct callinfo *c = stackI_callinfo(s); if(morphinem_unlikely(c == NULL)) throwI_message_error(s, "Require callable"); c; })

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
        stackI_ptr values;
        stackI_ptr space;
        stackI_ptr top;
    } s;

    size_t pop_size;

    size_t arguments_count;

    struct {
        size_t position;
        size_t callstate;
    } pc;

    struct {
        bool enable;
        size_t callstate;
        size_t space_size;
    } catch;

    bool exit;

    struct callinfo *prev;
};

struct stack {
    struct value *allocated;
    size_t top;
    size_t size;

    struct callinfo *callstack;
};

struct stack stackI_initial(morphine_instance_t, size_t grow);
void stackI_shrink(morphine_state_t);

struct value stackI_extract_callable(morphine_instance_t, struct value callable);

void stackI_call(
    morphine_state_t,
    struct value callable,
    struct value self,
    size_t argc,
    struct value *args,
    size_t pop_size
);

void stackI_call_pop(morphine_state_t);

void stackI_callinfo_free(morphine_instance_t, struct callinfo *callinfo);

size_t stackI_space_size(morphine_state_t);
void stackI_push(morphine_state_t, struct value value);
struct value stackI_peek(morphine_state_t, size_t offset);
struct value *stackI_vector(morphine_state_t, size_t offset, size_t size);
void stackI_pop(morphine_state_t, size_t count);
