//
// Created by why-iskra on 26.03.2024.
//

/*
 * {{docs header}}
 * path:vm/api-version
 * # Version API
 * ## Description
 * API for getting the virtual machine version
 * {{end}}
 */

#include "morphine/api.h"

/*
 * {{docs body}}
 * path:vm/api-version
 * ## mapi_version
 * ### Prototype
 * ```c
 * const char *mapi_version(void)
 * ```
 * ### Result
 * String of version
 * ### Description
 * Getting the virtual machine version
 * {{end}}
 */
MORPHINE_API const char *mapi_version(void) {
    return MORPHINE_VERSION;
}

/*
 * {{docs body}}
 * path:vm/api-version
 * ## mapi_version_code
 * ### Prototype
 * ```c
 * int mapi_version_code(void)
 * ```
 * ### Result
 * Version code
 * ### Description
 * Getting the virtual machine version code
 * {{end}}
 */
MORPHINE_API int mapi_version_code(void) {
    return MORPHINE_VERSION_CODE;
}