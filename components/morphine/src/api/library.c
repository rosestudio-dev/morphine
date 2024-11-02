//
// Created by why-iskra on 18.06.2024.
//

#include "morphine/api.h"
#include "morphine/core/libraries.h"
#include "morphine/core/instance.h"
#include "morphine/object/coroutine.h"

/*
 * {{docs body}}
 * path:architecture/api-instance
 * ## mapi_library_load
 * ### Prototype
 * ```c
 * void mapi_library_load(morphine_instance_t I, morphine_library_t library)
 * ```
 * ### Parameters
 * * `I` - instance
 * * `library` - library
 * ### Description
 * Load library
 * {{end}}
 */
MORPHINE_API void mapi_library_load(morphine_instance_t I, morphine_library_t library) {
    librariesI_load(I, library);
}

MORPHINE_API void mapi_library(morphine_coroutine_t U, const char *name) {
    struct value value = librariesI_get(U, name);
    stackI_push(U, value);
}
