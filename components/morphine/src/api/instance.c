//
// Created by whyiskra on 25.12.23.
//

/*
 * {{docs header}}
 * path:vm/api-instance
 * # Instance API
 * ## Description
 * API for working with instance
 * > See more [instance](instance), [interpreter](interpreter), [coroutines](coroutines)
 * {{end}}
 */

#include "morphine/api.h"
#include "morphine/core/instance.h"
#include "morphine/core/interpreter.h"
#include "morphine/object/coroutine.h"

/*
 * {{docs body}}
 * path:vm/api-instance
 * ## mapi_open
 * ### Prototype
 * ```c
 * morphine_instance_t mapi_open(morphine_platform_t P, morphine_settings_t S, void *data)
 * ```
 * ### Parameters
 * * `P` - platform
 * * `S` - settings
 * * `data` - data
 * ### Result
 * Pointer to instance
 * ### Description
 * Creates vm instance
 * {{end}}
 */
MORPHINE_API morphine_instance_t mapi_open(morphine_platform_t P, morphine_settings_t S, void *data) {
    return instanceI_open(P, S, data);
}

/*
 * {{docs body}}
 * path:vm/api-instance
 * ## mapi_instance_data
 * ### Prototype
 * ```c
 * void *mapi_instance_data(morphine_instance_t I)
 * ```
 * ### Parameters
 * * `I` - instance
 * ### Result
 * Pointer to data
 * ### Description
 * Gets pointer to data
 * {{end}}
 */
MORPHINE_API void *mapi_instance_data(morphine_instance_t I) {
    return I->data;
}

/*
 * {{docs body}}
 * path:vm/api-instance
 * ## mapi_interpreter
 * ### Prototype
 * ```c
 * void mapi_interpreter(morphine_instance_t I)
 * ```
 * ### Parameters
 * * `I` - instance
 * ### Description
 * Runs interpreter
 * {{end}}
 */
MORPHINE_API void mapi_interpreter(morphine_instance_t I) {
    interpreterI_run(I);
}

/*
 * {{docs body}}
 * path:vm/api-instance
 * ## mapi_interpreter_step
 * ### Prototype
 * ```c
 * bool mapi_interpreter_step(morphine_instance_t I)
 * ```
 * ### Parameters
 * * `I` - instance
 * ### Result
 * Interpreting is finished
 * ### Description
 * Runs one step of interpreter
 * {{end}}
 */
MORPHINE_API bool mapi_interpreter_step(morphine_instance_t I) {
    return interpreterI_step(I);
}

/*
 * {{docs body}}
 * path:vm/api-instance
 * ## mapi_close
 * ### Prototype
 * ```c
 * void mapi_close(morphine_instance_t I)
 * ```
 * ### Parameters
 * * `I` - instance
 * ### Description
 * Destruct vm instance
 * {{end}}
 */
MORPHINE_API void mapi_close(morphine_instance_t I) {
    instanceI_close(I);
}

/*
 * {{docs body}}
 * path:vm/api-instance
 * ## mapi_instance
 * ### Prototype
 * ```c
 * morphine_instance_t mapi_instance(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Result
 * Pointer to instance
 * ### Description
 * Gets pointer to instance from coroutine
 * {{end}}
 */
MORPHINE_API morphine_instance_t mapi_instance(morphine_coroutine_t U) {
    return U->I;
}

/*
 * {{docs body}}
 * path:vm/api-instance
 * ## mapi_coroutine
 * ### Prototype
 * ```c
 * morphine_coroutine_t mapi_coroutine(morphine_instance_t I)
 * ```
 * ### Parameters
 * * `I` - instance
 * ### Result
 * Pointer to coroutine
 * ### Description
 * Creates and attaches coroutine
 * {{end}}
 */
MORPHINE_API morphine_coroutine_t mapi_coroutine(morphine_instance_t I) {
    morphine_coroutine_t U = coroutineI_create(I, valueI_object(I->env));
    coroutineI_attach(U);
    return U;
}
