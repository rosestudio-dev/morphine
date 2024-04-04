//
// Created by whyiskra on 3/24/24.
//

#include "morphine/object/coroutine/stack/control.h"
#include "morphine/object/coroutine.h"
#include "morphine/gc/allocator.h"
#include "functions.h"

void stackI_shrink(morphine_coroutine_t U) {
    size_t grow = U->stack.settings.grow;

    if (U->stack.top + grow >= U->stack.size) {
        return;
    }

    size_t size = U->stack.top + grow;

    stack_save(&U->stack);

    U->stack.allocated = allocI_uni(U->I, U->stack.allocated, size * sizeof(struct value));

    stack_recover(&U->stack);

    U->stack.size = size;
}

void stackI_set_grow(morphine_coroutine_t U, size_t grow) {
    if (grow == 0) {
        throwI_error(U->I, "Stack grow size is zero");
    }

    U->stack.settings.grow = grow;
}

void stackI_set_limit(morphine_coroutine_t U, size_t limit) {
    if (limit == 0) {
        throwI_error(U->I, "Stack limit is zero");
    }

    U->stack.settings.limit = limit;
}