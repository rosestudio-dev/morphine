//
// Created by why-iskra on 04.04.2024.
//

#pragma once

#include "morphine/platform.h"
#include "morphine/core/callstack.h"

struct stack {
    struct {
        struct value *allocated;
        size_t top;
        size_t size;
    } array;

    size_t space_top;

    struct {
        size_t limit;
        size_t grow;
    } settings;

    struct {
        bool allow_shrinking;
    } control;
};

struct stack stackI_prototype(morphine_instance_t, size_t limit, size_t grow);
void stackI_destruct(morphine_instance_t, struct stack *);

void stackI_throw_fix(morphine_coroutine_t);

struct value *stackI_raise(morphine_coroutine_t, size_t size);
struct value *stackI_reduce(morphine_coroutine_t, size_t size);

void stackI_shrink(morphine_coroutine_t);

void stackI_set_grow(morphine_coroutine_t, size_t grow);
void stackI_set_limit(morphine_coroutine_t, size_t limit);

size_t stackI_space(morphine_coroutine_t);
void stackI_push(morphine_coroutine_t, struct value value);
struct value stackI_peek(morphine_coroutine_t, size_t offset);
void stackI_pop(morphine_coroutine_t, size_t count);
void stackI_rotate(morphine_coroutine_t, size_t count);
void stackI_replace(morphine_coroutine_t, size_t offset, struct value value);

struct value *stackI_unsafe_peek(morphine_coroutine_t, size_t offset);
