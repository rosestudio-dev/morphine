//
// Created by whyiskra on 3/24/24.
//

#include "functions.h"

void stack_recover(struct stack *stack) {
    struct callinfo *current = stack->callstack;
    while (current != NULL) {
        stack_ptr_recover(stack->allocated, current->s.callable);
        stack_ptr_recover(stack->allocated, current->s.source);
        stack_ptr_recover(stack->allocated, current->s.env);
        stack_ptr_recover(stack->allocated, current->s.self);
        stack_ptr_recover(stack->allocated, current->s.result);
        stack_ptr_recover(stack->allocated, current->s.thrown);
        stack_ptr_recover(stack->allocated, current->s.args);
        stack_ptr_recover(stack->allocated, current->s.slots);
        stack_ptr_recover(stack->allocated, current->s.params);
        stack_ptr_recover(stack->allocated, current->s.space);
        stack_ptr_recover(stack->allocated, current->s.top);

        current = current->prev;
    }
}