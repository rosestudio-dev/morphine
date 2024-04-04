//
// Created by whyiskra on 3/24/24.
//

#include "../functions.h"

#include "morphine/object/state.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"

stackI_ptr stack_raise(morphine_state_t S, size_t size) {
    if (size == 0) {
        throwI_error(S->I, "Raise size is zero");
    }

    struct stack *stack = &S->stack;

    stackI_ptr result = stack_ptr(stack->allocated + stack->top);

    if (size > SIZE_MAX - stack->top) {
        throwI_error(S->I, "Raise top overflow");
    }

    if (stack->top + size >= stack->size) {
        size_t grow = S->stack.settings.grow;
        size_t raise_size = ((size / grow) + 1) * grow;
        size_t new_size = stack->size + raise_size;

        if (raise_size > SIZE_MAX - stack->size) {
            throwI_error(S->I, "Raise size overflow");
        }

        if (new_size >= S->stack.settings.limit) {
            throwI_error(S->I, "Stack overflow");
        }

        stack_save(stack);
        stack_ptr_save(stack->allocated, result);

        stack->allocated = allocI_uni(S->I, stack->allocated, new_size * sizeof(struct value));

        stack_ptr_recover(stack->allocated, result);
        stack_recover(stack);

        stack->size = new_size;
    }

    stack->top += size;

    for (size_t i = stack->top - size; i < stack->top; i++) {
        stack->allocated[i] = valueI_nil;
    }

    return result;
}
