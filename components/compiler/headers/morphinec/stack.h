//
// Created by why-iskra on 02.06.2024.
//

#pragma once

#include <morphine.h>

#define stack_push_typed(T, U, S)   ((T *) stack_push((U), (S)))
#define stack_peek_typed(T, U, S)   ((T *) stack_peek((U), (S)))
#define stack_get_typed(T, U, S, i) ((T *) stack_get((U), (S), (i)))
#define stack_is_empty(S)           ((S).used == 0)
#define stack_size(S)               ((S).used)
#define stack_iterator(T, S, n)     for(T *n = (T *) (S).array; n < ((T *) ((S).array)) + (S).used; n ++)

struct stack {
    size_t type_size;
    size_t expansion_factor;
    size_t limit;
    size_t size;
    size_t used;
    void *array;
};

void stack_init(struct stack *, size_t type_size, size_t expansion_factor, size_t limit);
void stack_free(morphine_instance_t, struct stack *);
void *stack_push(morphine_coroutine_t, struct stack *);
void stack_pop(morphine_coroutine_t, struct stack *, size_t);
void *stack_peek(morphine_coroutine_t, struct stack *);
void *stack_get(morphine_coroutine_t, struct stack *, size_t);
