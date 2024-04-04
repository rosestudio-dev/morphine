//
// Created by whyiskra on 3/24/24.
//

#include "../functions.h"
#include "morphine/stack/access.h"

struct value stack_peek(morphine_state_t S, struct callinfo *callinfo, size_t offset) {
    struct value *p;
    size_t space_size;

    if (callinfo == NULL) {
        space_size = S->stack.top;
        p = S->stack.allocated + S->stack.top;
    } else {
        space_size = stack_space_size(S, callinfo);
        p = callinfo->s.top.p;
    }

    if (offset >= space_size) {
        throwI_error(S->I, "Cannot peek value from space");
    }

    return *(p - offset - 1);
}
