//
// Created by whyiskra on 3/24/24.
//

#include "../functions.h"
#include "morphine/stack/access.h"

struct value stack_peek(morphine_coroutine_t U, struct callinfo *callinfo, size_t offset) {
    struct value *p;
    size_t space_size;

    if (callinfo == NULL) {
        space_size = U->stack.top;
        p = U->stack.allocated + U->stack.top;
    } else {
        space_size = stack_space_size(U, callinfo);
        p = callinfo->s.top.p;
    }

    if (offset >= space_size) {
        throwI_error(U->I, "Cannot peek value from space");
    }

    return *(p - offset - 1);
}
