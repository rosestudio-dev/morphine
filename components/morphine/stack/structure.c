//
// Created by whyiskra on 3/24/24.
//

#include "morphine/stack/structure.h"
#include "morphine/core/allocator.h"
#include "morphine/stack/call.h"

struct stack stackI_initial(morphine_instance_t I, size_t limit, size_t grow) {
    if (grow == 0) {
        throwI_message_panic(I, NULL, "Stack grow size is zero");
    }

    if (limit == 0) {
        throwI_message_panic(I, NULL, "Stack limit is zero");
    }

    return (struct stack) {
        .allocated = NULL,
        .size = 0,
        .top = 0,
        .settings.grow = grow,
        .settings.limit = limit,
        .callstack = NULL
    };
}

void stackI_destruct(morphine_instance_t I, struct stack stack) {
    struct callinfo *callinfo = stack.callstack;
    while (callinfo != NULL) {
        struct callinfo *prev = callinfo->prev;
        callstackI_info_free(I, callinfo);
        callinfo = prev;
    }
    stack.callstack = NULL;

    allocI_free(I, stack.allocated);
}

size_t stackI_allocated_size(struct stack stack) {
    size_t size = sizeof(struct value) * stack.size;

    struct callinfo *callinfo = stack.callstack;
    while (callinfo != NULL) {
        size += sizeof(struct callinfo);
        callinfo = callinfo->prev;
    }

    return size;
}