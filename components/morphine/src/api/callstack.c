//
// Created by whyiskra on 25.12.23.
//

/*
 * {{docs header}}
 * path:architecture/api-callstack
 * # Callstack API
 * ## Description
 * API for working with callstack
 * > See more [call](call)
 * {{end}}
 */

#include "morphine/api.h"
#include "morphine/object/string.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/value.h"
#include "morphine/core/throw.h"

MORPHINE_API void mapi_call(morphine_coroutine_t U, ml_size argc) {
    callstackI_call_from_api(U, false, false, argc);
}

MORPHINE_API void mapi_ecall(morphine_coroutine_t U, ml_size argc) {
    callstackI_call_from_api(U, true, false, argc);
}

MORPHINE_API void mapi_scall(morphine_coroutine_t U, ml_size argc) {
    callstackI_call_from_api(U, false, true, argc);
}

MORPHINE_API void mapi_escall(morphine_coroutine_t U, ml_size argc) {
    callstackI_call_from_api(U, true, true, argc);
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_push_callable
 * ### Prototype
 * ```c
 * void mapi_push_callable(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Pushes the callable onto the stack
 * {{end}}
 */
MORPHINE_API void mapi_push_callable(morphine_coroutine_t U) {
    stackI_push(U, *callstackI_info_or_error(U)->s.direct.callable);
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_extract_callable
 * ### Prototype
 * ```c
 * void mapi_extract_callable(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Extracts a peeked callable and pushes it onto the stack
 * {{end}}
 */
MORPHINE_API void mapi_extract_callable(morphine_coroutine_t U) {
    stackI_push(U, callstackI_extract_callable(U->I, stackI_peek(U, 0)));
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_push_result
 * ### Prototype
 * ```c
 * void mapi_push_result(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Pushes the result of the call onto the stack (if there were no calls, then nil)
 * {{end}}
 */
MORPHINE_API void mapi_push_result(morphine_coroutine_t U) {
    stackI_push(U, callstackI_result(U));
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_return
 * ### Prototype
 * ```c
 * void mapi_return(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Pops the return value from the stack and leaves
 * {{end}}
 */
MORPHINE_API void mapi_return(morphine_coroutine_t U) {
    callstackI_return(U, stackI_peek(U, 0));
    stackI_pop(U, 1);
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_leave
 * ### Prototype
 * ```c
 * void mapi_leave(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Leaves with a nil return value
 * {{end}}
 */
MORPHINE_API void mapi_leave(morphine_coroutine_t U) {
    callstackI_return(U, valueI_nil);
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_continue
 * ### Prototype
 * ```c
 * void mapi_continue(morphine_coroutine_t U, size_t callstate)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * * `callstate` - callstate
 * ### Description
 * Set callstate
 * {{end}}
 */
MORPHINE_API void mapi_continue(morphine_coroutine_t U, size_t callstate) {
    callstackI_continue(U, callstate);
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_callstate
 * ### Prototype
 * ```c
 * size_t mapi_callstate(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Result
 * Callstate
 * ### Description
 * Gets callstate
 * {{end}}
 */
MORPHINE_API size_t mapi_callstate(morphine_coroutine_t U) {
    return callstackI_state(U);
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_args
 * ### Prototype
 * ```c
 * ml_size mapi_args(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Result
 * Count of arguments
 * ### Description
 * Gets count of arguments
 * {{end}}
 */
MORPHINE_API ml_size mapi_args(morphine_coroutine_t U) {
    return callstackI_info_or_error(U)->info.arguments_count;
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_push_arg
 * ### Prototype
 * ```c
 * void mapi_push_arg(morphine_coroutine_t U, ml_size index)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * * `index` - index of argument
 * ### Description
 * Pushes the argument onto the stack
 * {{end}}
 */
MORPHINE_API void mapi_push_arg(morphine_coroutine_t U, ml_size index) {
    struct callinfo *callinfo = callstackI_info_or_error(U);
    if (index >= callinfo->info.arguments_count) {
        throwI_error(U->I, "argument index out of bounce");
    }

    stackI_push(U, callinfo->s.direct.args[index]);
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_push_env
 * ### Prototype
 * ```c
 * void mapi_push_env(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Pushes the env onto the stack
 * {{end}}
 */
MORPHINE_API void mapi_push_env(morphine_coroutine_t U) {
    struct value env;
    if (callstackI_info(U) != NULL) {
        env = *callstackI_info(U)->s.direct.env;
    } else {
        env = U->env;
    }

    stackI_push(U, env);
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_push_self
 * ### Prototype
 * ```c
 * void mapi_push_self(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Pushes the self value onto the stack
 * {{end}}
 */
MORPHINE_API void mapi_push_self(morphine_coroutine_t U) {
    stackI_push(U, *callstackI_info_or_error(U)->s.direct.self);
}
