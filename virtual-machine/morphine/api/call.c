//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/core/call.h"
#include "morphine/core/init.h"

MORPHINE_API void mapi_call(morphine_state_t S, size_t argc) {
    struct value callable = stackI_peek(S, argc);
    struct value *args = stackI_vector(S, 0, argc);
    callI_do(S, callable, valueI_nil, argc, args, argc + 1);
}

MORPHINE_API void mapi_calli(morphine_state_t S, size_t argc) {
    struct value *args = stackI_vector(S, 1, argc);
    struct value callable = stackI_peek(S, 0);
    callI_do(S, callable, valueI_nil, argc, args, argc + 1);
}

MORPHINE_API void mapi_callself(morphine_state_t S, size_t argc) {
    struct value self = stackI_peek(S, argc + 1);
    struct value callable = stackI_peek(S, argc);
    struct value *args = stackI_vector(S, 0, argc);
    callI_do(S, callable, self, argc, args, argc + 2);
}

MORPHINE_API void mapi_callselfi(morphine_state_t S, size_t argc) {
    struct value *args = stackI_vector(S, 2, argc);
    struct value callable = stackI_peek(S, 1);
    struct value self = stackI_peek(S, 0);
    callI_do(S, callable, self, argc, args, argc + 2);
}

MORPHINE_API void mapi_push_result(morphine_state_t S) {
    stackI_push(S, callI_result(S));
}

MORPHINE_API void mapi_return(morphine_state_t S) {
    callI_return(S, stackI_peek(S, 0));
    stackI_pop(S, 1);
}

MORPHINE_API void mapi_leave(morphine_state_t S) {
    callI_leave(S);
}

MORPHINE_API void mapi_continue(morphine_state_t S, size_t callstate) {
    callI_continue(S, callstate);
}

MORPHINE_API size_t mapi_callstate(morphine_state_t S) {
    return callI_callstate(S);
}

MORPHINE_API size_t mapi_args(morphine_state_t S) {
    return stackI_callinfo_or_error(S)->arguments_count;
}

MORPHINE_API void mapi_push_arg(morphine_state_t S, size_t index) {
    struct callinfo *callinfo = stackI_callinfo_or_error(S);
    if (index >= callinfo->arguments_count) {
        throwI_message_error(S, "Argument index out of bounce");
    }

    stackI_push(S, callinfo->s.args.p[index]);
}

MORPHINE_API void mapi_push_env(morphine_state_t S) {
    stackI_push(S, *stackI_callinfo_or_error(S)->s.env.p);
}

MORPHINE_API void mapi_push_self(morphine_state_t S) {
    stackI_push(S, *stackI_callinfo_or_error(S)->s.self.p);
}

MORPHINE_API void mapi_changeenv(morphine_state_t S) {
    struct value env = stackI_peek(S, 0);
    *stackI_callinfo_or_error(S)->s.env.p = env;
    stackI_pop(S, 0);
}
