//
// Created by whyiskra on 3/24/24.
//

#pragma once

#include "morphine/platform.h"

struct stack {
    struct value *allocated;
    size_t top;
    size_t size;

    struct {
        size_t limit;
        size_t grow;
    } settings;

    size_t callstack_size;
    struct callinfo *callstack;
};

struct stack stackI_initial(morphine_instance_t, size_t limit, size_t grow);
void stackI_destruct(morphine_instance_t, struct stack);

size_t stackI_allocated_size(struct stack stack);
