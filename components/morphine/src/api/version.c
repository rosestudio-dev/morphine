//
// Created by why-iskra on 26.03.2024.
//

/*
 * {{docs header}}
 * path:architecture/api-version
 * # Version API
 * ## Description
 * API for versions
 * {{end}}
 */

#include "morphine/api.h"

/*
 * {{docs body}}
 * path:architecture/api-version
 * ## mapi_version_name
 * ### Prototype
 * ```c
 * const char *mapi_version_name(void)
 * ```
 * ### Result
 * Name of version
 * ### Description
 * Getting name of the virtual machine version
 * {{end}}
 */
MORPHINE_API const char *mapi_version_name(void) {
    return MORPHINE_VERSION_NAME;
}

/*
 * {{docs body}}
 * path:architecture/api-version
 * ## mapi_version
 * ### Prototype
 * ```c
 * ml_version mapi_version(void)
 * ```
 * ### Result
 * Version
 * ### Description
 * Getting the virtual machine version
 * {{end}}
 */
MORPHINE_API ml_version mapi_version(void) {
    return MORPHINE_VERSION_CODE;
}

/*
 * {{docs body}}
 * path:architecture/api-version
 * ## mapi_bytecode_version
 * ### Prototype
 * ```c
 * ml_version mapi_bytecode_version(void)
 * ```
 * ### Result
 * Bytecode version
 * ### Description
 * Getting the bytecode version
 * {{end}}
 */
MORPHINE_API ml_version mapi_bytecode_version(void) {
    return MORPHINE_BYTECODE_VERSION;
}