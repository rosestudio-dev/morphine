//
// Created by why-iskra on 02.06.2024.
//

#pragma once

#include <morphine.h>

#define stack_is_empty(S)           ((S).used == 0)
#define stack_size(S)               ((S).used)
#define stack_iterator(S, n)        for(size_t n = 0; (n) < (S).used; (n) ++)
#define stack_idx_invert(S, n)      ((S).used - (n) - 1)
#define stack_it(S, n)              (((S).array) + (n))
#define stack_it_invert(S, n)       (((S).array) + stack_idx_invert(S, n))

#define define_stack(n, t) define_stack_struct(n, t) \
                           define_stack_init(n, t) \
                           define_stack_free(n)

#define define_stack_struct(n, t) struct stack_##n { \
    size_t expansion_factor; \
    size_t limit; \
    size_t size; \
    size_t used; \
    t *array; \
};

#define define_stack_init(n, t) static void stack_##n##_init(struct stack_##n *S, size_t factor, size_t limit) { \
    *S = (struct stack_##n) { \
        .expansion_factor = factor, \
        .limit = limit, \
        .size = 0, \
        .used = 0, \
        .array = NULL \
    }; \
}

#define define_stack_free(n) static void stack_##n##_free(morphine_instance_t I, struct stack_##n *S) { mapi_allocator_free(I, S->array); }

#define define_stack_push(n, t) static t *stack_##n##_push(morphine_coroutine_t U, struct stack_##n *S) { \
    if (S->used == S->size) { \
        if (S->size >= S->limit) { mapi_error(U, #n" stack too big"); } \
        S->array = mapi_allocator_vec(mapi_instance(U), S->array, S->size + S->expansion_factor, sizeof(t)); \
        S->size += S->expansion_factor; \
    } \
    S->used ++; \
    return S->array + S->used - 1; \
}

#define define_stack_pop(n, t) static void stack_##n##_pop(morphine_coroutine_t U, struct stack_##n *S, size_t count) { \
    if (count > S->used) { mapi_error(U, "cannot pop from "#n" stack"); } \
    S->used -= count; \
}

#define define_stack_peek(n, t) static t *stack_##n##_peek(morphine_coroutine_t U, struct stack_##n *S) { \
    if (stack_is_empty(*S)) { mapi_error(U, #n" stack is empty"); } \
    return S->array + S->used - 1; \
}

#define define_stack_get(n, t) static t *stack_##n##_get(morphine_coroutine_t U, struct stack_##n *S, size_t index) { \
    if (stack_is_empty(*S)) { mapi_error(U, #n" stack is empty"); } \
    if (index >= stack_size(*S)) { mapi_error(U, "index of "#n" stack is out of bounce"); } \
    return S->array + index; \
}
