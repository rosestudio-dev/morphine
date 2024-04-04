//
// Created by whyiskra on 25.12.23.
//

#include "morphine/api.h"
#include "morphine/object/string.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/value.h"
#include "morphine/core/throw.h"

MORPHINE_API void mapi_call(morphine_coroutine_t U, size_t argc) {
    struct value callable = stackI_peek(U, argc);
    callstackI_call_stack(U, callable, valueI_nil, 0, argc, argc + 1);
}

MORPHINE_API void mapi_calli(morphine_coroutine_t U, size_t argc) {
    struct value callable = stackI_peek(U, 0);
    callstackI_call_stack(U, callable, valueI_nil, 1, argc, argc + 1);
}

MORPHINE_API void mapi_callself(morphine_coroutine_t U, size_t argc) {
    struct value self = stackI_peek(U, argc + 1);
    struct value callable = stackI_peek(U, argc);
    callstackI_call_stack(U, callable, self, 0, argc, argc + 2);
}

MORPHINE_API void mapi_callselfi(morphine_coroutine_t U, size_t argc) {
    struct value callable = stackI_peek(U, 1);
    struct value self = stackI_peek(U, 0);
    callstackI_call_stack(U, callable, self, 2, argc, argc + 2);
}

MORPHINE_API void mapi_push_result(morphine_coroutine_t U) {
    stackI_push(U, callstackI_result(U));
}

MORPHINE_API void mapi_return(morphine_coroutine_t U) {
    callstackI_return(U, stackI_peek(U, 0));
    stackI_pop(U, 1);
}

MORPHINE_API void mapi_leave(morphine_coroutine_t U) {
    callstackI_leave(U);
}

MORPHINE_API void mapi_continue(morphine_coroutine_t U, size_t callstate) {
    callstackI_continue(U, callstate);
}

MORPHINE_API size_t mapi_callstate(morphine_coroutine_t U) {
    return callstackI_state(U);
}

MORPHINE_API size_t mapi_args(morphine_coroutine_t U) {
    return callstackI_info_or_error(U)->arguments_count;
}

MORPHINE_API void mapi_push_arg(morphine_coroutine_t U, size_t index) {
    struct callinfo *callinfo = callstackI_info_or_error(U);
    if (index >= callinfo->arguments_count) {
        throwI_error(U->I, "Argument index out of bounce");
    }

    stackI_push(U, callinfo->s.args.p[index]);
}

MORPHINE_API void mapi_push_env(morphine_coroutine_t U) {
    stackI_push(U, *callstackI_info_or_error(U)->s.env.p);
}

MORPHINE_API void mapi_push_self(morphine_coroutine_t U) {
    stackI_push(U, *callstackI_info_or_error(U)->s.self.p);
}

MORPHINE_API void mapi_changeenv(morphine_coroutine_t U) {
    struct value env = stackI_peek(U, 0);
    *callstackI_info_or_error(U)->s.env.p = env;
}
