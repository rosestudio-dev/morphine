//
// Created by whyiskra on 30.12.23.
//

#include "morphine/api.h"
#include "morphine/object/state.h"
#include "morphine/core/throw.h"
#include "morphine/stack/access.h"

MORPHINE_API morphine_state_t mapi_push_state(morphine_state_t S) {
    morphine_state_t state = stateI_create(S->I);
    stackI_push(S, valueI_object(state));

    return state;
}

MORPHINE_API void mapi_push_current_state(morphine_state_t S) {
    stackI_push(S, valueI_object(S));
}

MORPHINE_API morphine_state_t mapi_get_state(morphine_state_t S) {
    return valueI_as_state_or_error(S->I, stackI_peek(S, 0));
}

MORPHINE_API void mapi_attach(morphine_state_t S) {
    stateI_attach(S);
}

MORPHINE_API void mapi_state_suspend(morphine_state_t S) {
    stateI_suspend(S);
}

MORPHINE_API void mapi_state_kill(morphine_state_t S) {
    stateI_kill(S);
}

MORPHINE_API void mapi_state_resume(morphine_state_t S) {
    stateI_resume(S);
}

MORPHINE_API void mapi_state_priority(morphine_state_t S, priority_t priority) {
    stateI_priority(S, priority);
}

MORPHINE_API const char *mapi_state_status(morphine_state_t S) {
    return stateI_status2string(S, S->status);
}

MORPHINE_API bool mapi_state_checkstatus(morphine_state_t S, const char *name) {
    return S->status == stateI_string2status(S, name);
}

MORPHINE_API bool mapi_state_isalive(morphine_state_t S) {
    return stateI_isalive(S);
}
