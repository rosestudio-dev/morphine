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

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_call
 * ### Prototype
 * ```c
 * void mapi_call(morphine_coroutine_t U, ml_size argc)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * * `argc` - count of arguments
 * ### Description
 * 1. Peeks the specified count of arguments from the stack
 * 2. Calls the callable value at the top of the stack with offset of argc (self value is nil)
 *
 * After the call completes, pops argc + 1 values from the stack
 *
 * > [!WARNING]
 * > After calling this api function, the coroutine switches to another call
 * {{end}}
 */
MORPHINE_API void mapi_call(morphine_coroutine_t U, ml_size argc) {
    struct value callable = stackI_peek(U, argc);
    callstackI_call_from_api(U, callable, NULL, 0, argc, ((size_t) argc) + 1);
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_calli
 * ### Prototype
 * ```c
 * void mapi_calli(morphine_coroutine_t U, ml_size argc)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * * `argc` - count of arguments
 * ### Description
 * 1. Peeks the specified count of arguments from the stack with offset of 1
 * 2. Calls the callable value at the top of the stack (self value is nil)
 *
 * After the call completes, pops argc + 1 values from the stack
 *
 * > [!WARNING]
 * > After calling this api function, the coroutine switches to another call
 * {{end}}
 */
MORPHINE_API void mapi_calli(morphine_coroutine_t U, ml_size argc) {
    struct value callable = stackI_peek(U, 0);
    callstackI_call_from_api(U, callable, NULL, 1, argc, ((size_t) argc) + 1);
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_callself
 * ### Prototype
 * ```c
 * void mapi_callself(morphine_coroutine_t U, ml_size argc)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * * `argc` - count of arguments
 * ### Description
 * 1. Peeks the specified count of arguments from the stack
 * 2. Calls the callable value at the top of the stack with offset of argc (self value is peek from the stack with offset of argc + 1)
 *
 * After the call completes, pops argc + 2 values from the stack
 *
 * > [!WARNING]
 * > After calling this api function, the coroutine switches to another call
 * {{end}}
 */
MORPHINE_API void mapi_callself(morphine_coroutine_t U, ml_size argc) {
    struct value self = stackI_peek(U, argc + 1);
    struct value callable = stackI_peek(U, argc);
    callstackI_call_from_api(U, callable, &self, 0, argc, ((size_t) argc) + 2);
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_callselfi
 * ### Prototype
 * ```c
 * void mapi_callselfi(morphine_coroutine_t U, ml_size argc)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * * `argc` - count of arguments
 * ### Description
 * 1. Peeks the specified count of arguments from the stack with offset of 2
 * 2. Calls the callable value at the top of the stack with offset of 1 (self value is peek from the stack with offset of 1)
 *
 * After the call completes, pops argc + 2 values from the stack
 *
 * > [!WARNING]
 * > After calling this api function, the coroutine switches to another call
 * {{end}}
 */
MORPHINE_API void mapi_callselfi(morphine_coroutine_t U, ml_size argc) {
    struct value callable = stackI_peek(U, 1);
    struct value self = stackI_peek(U, 0);
    callstackI_call_from_api(U, callable, &self, 2, argc, ((size_t) argc) + 2);
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
    stackI_push(U, *callstackI_info_or_error(U)->s.callable);
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
    callstackI_leave(U);
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
    return callstackI_info_or_error(U)->arguments_count;
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
    if (index >= callinfo->arguments_count) {
        throwI_error(U->I, "argument index out of bounce");
    }

    stackI_push(U, callinfo->s.args[index]);
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
        env = *callstackI_info(U)->s.env;
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
    stackI_push(U, *callstackI_info_or_error(U)->s.self);
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_change_env
 * ### Prototype
 * ```c
 * void mapi_change_env(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Pops a value from the stack and sets it to env
 * {{end}}
 */
MORPHINE_API void mapi_change_env(morphine_coroutine_t U) {
    struct value env = stackI_peek(U, 0);
    *callstackI_info_or_error(U)->s.env = env;
}

/*
 * {{docs body}}
 * path:architecture/api-callstack
 * ## mapi_bind_registry
 * ### Prototype
 * ```c
 * void mapi_bind_registry(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Binds call info as raw value with registry (after call it will automatically delete from registry)
 * {{end}}
 */
MORPHINE_API void mapi_bind_registry(morphine_coroutine_t U) {
    callstackI_bind_registry(U);
}
