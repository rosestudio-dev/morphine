//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/core/init.h"
#include "morphine/stack/access.h"
#include "morphine/stack/call.h"

MORPHINE_API void mapi_call(morphine_state_t S, size_t argc) {
    struct value callable = stackI_peek(S, argc);
    callstackI_stack(S, callable, valueI_nil, 0, argc, argc + 1);
}

MORPHINE_API void mapi_calli(morphine_state_t S, size_t argc) {
    struct value callable = stackI_peek(S, 0);
    callstackI_stack(S, callable, valueI_nil, 1, argc, argc + 1);
}

MORPHINE_API void mapi_callself(morphine_state_t S, size_t argc) {
    struct value self = stackI_peek(S, argc + 1);
    struct value callable = stackI_peek(S, argc);
    callstackI_stack(S, callable, self, 0, argc, argc + 2);
}

MORPHINE_API void mapi_callselfi(morphine_state_t S, size_t argc) {
    struct value callable = stackI_peek(S, 1);
    struct value self = stackI_peek(S, 0);
    callstackI_stack(S, callable, self, 2, argc, argc + 2);
}

MORPHINE_API void mapi_push_result(morphine_state_t S) {
    stackI_push(S, callstackI_result(S));
}

MORPHINE_API void mapi_return(morphine_state_t S) {
    callstackI_return(S, stackI_peek(S, 0));
    stackI_pop(S, 1);
}

MORPHINE_API void mapi_leave(morphine_state_t S) {
    callstackI_leave(S);
}

MORPHINE_API void mapi_continue(morphine_state_t S, size_t callstate) {
    callstackI_continue(S, callstate);
}

MORPHINE_API size_t mapi_callstate(morphine_state_t S) {
    return callstackI_state(S);
}

MORPHINE_API size_t mapi_args(morphine_state_t S) {
    return callstackI_info_or_error(S)->arguments_count;
}

MORPHINE_API void mapi_push_arg(morphine_state_t S, size_t index) {
    struct callinfo *callinfo = callstackI_info_or_error(S);
    if (index >= callinfo->arguments_count) {
        throwI_message_error(S, "Argument index out of bounce");
    }

    stackI_push(S, callinfo->s.args.p[index]);
}

MORPHINE_API void mapi_push_env(morphine_state_t S) {
    stackI_push(S, *callstackI_info_or_error(S)->s.env.p);
}

MORPHINE_API void mapi_push_self(morphine_state_t S) {
    stackI_push(S, *callstackI_info_or_error(S)->s.self.p);
}

MORPHINE_API void mapi_changeenv(morphine_state_t S) {
    struct value env = stackI_peek(S, 0);
    *callstackI_info_or_error(S)->s.env.p = env;
    stackI_pop(S, 0);
}
