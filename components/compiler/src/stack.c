//
// Created by why-iskra on 02.06.2024.
//

#include "morphinec/stack.h"

void stack_init(struct stack *S, size_t type_size, size_t expansion_factor, size_t limit) {
    *S = (struct stack) {
        .type_size = type_size,
        .expansion_factor = expansion_factor,
        .limit = limit,
        .size = 0,
        .used = 0,
        .array = NULL
    };
}

void stack_free(morphine_instance_t I, struct stack *S) {
    mapi_allocator_free(I, S->array);
}

void *stack_push(morphine_coroutine_t U, struct stack *S) {
    if (S->used == S->size) {
        if (S->size >= S->limit) {
            mapi_error(U, "stack too big");
        }

        S->array = mapi_allocator_vec(
            mapi_instance(U),
            S->array,
            S->size + S->expansion_factor,
            S->type_size
        );

        S->size += S->expansion_factor;
    }

    void *result = S->array + (S->type_size * S->used);
    S->used++;

    return result;
}

void stack_pop(morphine_coroutine_t U, struct stack *S, size_t count) {
    if (count > S->used) {
        mapi_error(U, "cannot pop from stack");
    }

    S->used -= count;
}

void *stack_peek(morphine_coroutine_t U, struct stack *S) {
    if (stack_is_empty(*S)) {
        mapi_error(U, "stack is empty");
    }

    return S->array + (S->type_size * (S->used - 1));
}

void *stack_get(morphine_coroutine_t U, struct stack *S, size_t index) {
    if (stack_is_empty(*S)) {
        mapi_error(U, "stack is empty");
    }

    if (index >= stack_size(*S)) {
        mapi_error(U, "index is out of bounce");
    }

    return S->array + (S->type_size * index);
}