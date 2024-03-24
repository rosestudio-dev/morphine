//
// Created by why on 3/24/24.
//

#pragma once

#include "morphine/stack/structure.h"
#include "morphine/stack/call.h"

#define stack_ptr(ptr) ((stackI_ptr) { .p = (ptr) })
#define stack_ptr_save(a, ptr) (ptr) = (stackI_ptr) { .diff = (size_t) ((ptr).p - (a)) }
#define stack_ptr_recover(a, ptr) (ptr) = (stackI_ptr) { .p = (a) + (ptr).diff }

void stack_save(struct stack *);
void stack_recover(struct stack *);

stackI_ptr stack_raise(morphine_state_t, struct value first, size_t size);
stackI_ptr stack_reduce(morphine_state_t, size_t size);

struct value stack_peek(morphine_state_t, struct callinfo *, size_t offset);
size_t stack_space_size(morphine_state_t, struct callinfo *);
