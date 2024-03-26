//
// Created by whyiskra on 1/28/24.
//

#include "morphine/auxiliary.h"
#include "morphine/api.h"

static void empty(morphine_state_t S) {
    mapi_leave(S);
}

MORPHINE_AUX void maux_push_empty_callable(morphine_state_t S) {
    mapi_push_native(S, "aux.empty", empty);
}
