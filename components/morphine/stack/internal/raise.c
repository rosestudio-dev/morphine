//
// Created by whyiskra on 3/24/24.
//

#include "functions.h"

#include "morphine/object/state.h"
#include "morphine/core/throw.h"
#include "morphine/core/allocator.h"

stackI_ptr stack_raise(morphine_state_t S, struct value first, size_t size) {
    if (size == 0) {
        throwI_message_panic(S->I, S, "Raise size is zero");
    }

    struct stack *stack = &S->stack;

    stackI_ptr result = stack_ptr(stack->allocated + stack->top);
    *result.p = first;

    if (stack->top + size >= stack->size) {
        size_t grow = S->stack.settings.grow;
        size_t new_size = stack->size + (((size / grow) + 1) * grow);

        if (new_size >= S->stack.settings.limit) {
            throwI_message_error(S, "Stack overflow");
        }

        stack_save(stack);
        stack_ptr_save(stack->allocated, result);

        stack->allocated = allocI_uni(S->I, stack->allocated, new_size * sizeof(struct value));

        stack_ptr_recover(stack->allocated, result);
        stack_recover(stack);

        stack->size = new_size;
    }

    stack->top += size;

    for (size_t i = stack->top - size + 1; i < stack->top; i++) {
        stack->allocated[i] = valueI_nil;
    }

    return result;
}
