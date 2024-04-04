//
// Created by whyiskra on 3/24/24.
//

#include "../functions.h"

size_t stack_space_size(morphine_coroutine_t U, struct callinfo *callinfo) {
    if (callinfo == NULL) {
        return U->stack.top;
    }

    return (size_t) (callinfo->s.top.p - callinfo->s.space.p);
}