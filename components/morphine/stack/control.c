//
// Created by whyiskra on 3/24/24.
//

#include "morphine/stack/control.h"
#include "morphine/object/state.h"
#include "morphine/gc/allocator.h"
#include "functions.h"

void stackI_shrink(morphine_state_t S) {
    size_t grow = S->stack.settings.grow;

    if (S->stack.top + grow >= S->stack.size) {
        return;
    }

    size_t size = S->stack.top + grow;

    stack_save(&S->stack);

    S->stack.allocated = allocI_uni(S->I, S->stack.allocated, size * sizeof(struct value));

    stack_recover(&S->stack);

    S->stack.size = size;
}

void stackI_set_grow(morphine_state_t S, size_t grow) {
    if (grow == 0) {
        throwI_error(S->I, "Stack grow size is zero");
    }

    S->stack.settings.grow = grow;
}

void stackI_set_limit(morphine_state_t S, size_t limit) {
    if (limit == 0) {
        throwI_error(S->I, "Stack limit is zero");
    }

    S->stack.settings.limit = limit;
}