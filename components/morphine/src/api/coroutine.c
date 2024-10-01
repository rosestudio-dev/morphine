//
// Created by whyiskra on 30.12.23.
//

/*
 * {{docs header}}
 * path:architecture/api-coroutine
 * # Coroutine API
 * ## Description
 * API for working with coroutines
 * > See more: [coroutines](coroutines), [interpreter](interpreter)
 * {{end}}
 */

#include "morphine/api.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/throw.h"

/*
 * {{docs body}}
 * path:architecture/api-coroutine
 * ## mapi_push_coroutine
 * ### Prototype
 * ```c
 * morphine_coroutine_t mapi_push_coroutine(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Result
 * Pointer to coroutine
 * ### Description
 * Pops coroutine name, creates a coroutine, pushes it onto the stack and returns pointer to it
 * {{end}}
 */
MORPHINE_API morphine_coroutine_t mapi_push_coroutine(morphine_coroutine_t U) {
    struct string *name = valueI_as_string_or_error(U->I, stackI_peek(U, 0));

    struct value env;
    if (callstackI_info(U) != NULL) {
        env = *callstackI_info(U)->s.direct.env;
    } else {
        env = U->env;
    }

    morphine_coroutine_t coroutine = coroutineI_create(U->I, name, env);
    stackI_replace(U, 0, valueI_object(coroutine));

    return coroutine;
}

/*
 * {{docs body}}
 * path:architecture/api-coroutine
 * ## mapi_get_coroutine
 * ### Prototype
 * ```c
 * morphine_coroutine_t mapi_get_coroutine(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Result
 * Pointer to coroutine
 * ### Description
 * Peeks the coroutine from the stack and returns pointer to it
 * {{end}}
 */
MORPHINE_API morphine_coroutine_t mapi_get_coroutine(morphine_coroutine_t U) {
    return valueI_as_coroutine_or_error(U->I, stackI_peek(U, 0));
}

/*
 * {{docs body}}
 * path:architecture/api-coroutine
 * ## mapi_current
 * ### Prototype
 * ```c
 * void mapi_current(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Pushes current coroutine onto the stack
 * {{end}}
 */
MORPHINE_API void mapi_current(morphine_coroutine_t U) {
    stackI_push(U, valueI_object(U));
}

/*
 * {{docs body}}
 * path:architecture/api-coroutine
 * ## mapi_attach
 * ### Prototype
 * ```c
 * void mapi_attach(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Attaches the coroutine to the interpreter's loop
 * {{end}}
 */
MORPHINE_API void mapi_attach(morphine_coroutine_t U) {
    coroutineI_attach(U);
}

/*
 * {{docs body}}
 * path:architecture/api-coroutine
 * ## mapi_coroutine_suspend
 * ### Prototype
 * ```c
 * void mapi_coroutine_suspend(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Suspends the coroutine
 * {{end}}
 */
MORPHINE_API void mapi_coroutine_suspend(morphine_coroutine_t U) {
    coroutineI_suspend(U);
}

/*
 * {{docs body}}
 * path:architecture/api-coroutine
 * ## mapi_coroutine_kill
 * ### Prototype
 * ```c
 * void mapi_coroutine_kill(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Kills the coroutine
 * {{end}}
 */
MORPHINE_API void mapi_coroutine_kill(morphine_coroutine_t U) {
    coroutineI_kill(U);
}

/*
 * {{docs body}}
 * path:architecture/api-coroutine
 * ## mapi_coroutine_resume
 * ### Prototype
 * ```c
 * void mapi_coroutine_resume(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Resumes the coroutine
 * {{end}}
 */
MORPHINE_API void mapi_coroutine_resume(morphine_coroutine_t U) {
    coroutineI_resume(U);
}

/*
 * {{docs body}}
 * path:architecture/api-coroutine
 * ## mapi_coroutine_priority
 * ### Prototype
 * ```c
 * void mapi_coroutine_priority(morphine_coroutine_t U, ml_size priority)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * * `priority` - priority (from 1)
 * ### Description
 * Changes the coroutine's priority
 * {{end}}
 */
MORPHINE_API void mapi_coroutine_priority(morphine_coroutine_t U, ml_size priority) {
    coroutineI_priority(U, priority);
}

/*
 * {{docs body}}
 * path:architecture/api-coroutine
 * ## mapi_coroutine_status
 * ### Prototype
 * ```c
 * const char *mapi_coroutine_status(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Result
 * String of status
 * ### Description
 * Gets status of the coroutine
 * {{end}}
 */
MORPHINE_API const char *mapi_coroutine_status(morphine_coroutine_t U) {
    return coroutineI_status2string(U, U->state.status);
}

/*
 * {{docs body}}
 * path:architecture/api-coroutine
 * ## mapi_coroutine_is_alive
 * ### Prototype
 * ```c
 * bool mapi_coroutine_is_alive(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Result
 * Coroutine is alive
 * ### Description
 * Checks the coroutine is alive
 * {{end}}
 */
MORPHINE_API bool mapi_coroutine_is_alive(morphine_coroutine_t U) {
    return coroutineI_isalive(U);
}

/*
 * {{docs body}}
 * path:architecture/api-coroutine
 * ## mapi_coroutine_name
 * ### Prototype
 * ```c
 * void mapi_coroutine_name(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Pushes the coroutine name into stack
 * {{end}}
 */
MORPHINE_API void mapi_coroutine_name(morphine_coroutine_t U) {
    stackI_push(U, valueI_object(U->name));
}
