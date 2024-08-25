//
// Created by whyiskra on 3/19/24.
//

/*
 * {{docs header}}
 * path:architecture/api-allocator
 * # Allocator API
 * ## Description
 * API for working with memory
 * > See more [garbage collector](garbage-collector)
 * {{end}}
 */

#include "morphine/api.h"
#include "morphine/gc/allocator.h"

/*
 * {{docs body}}
 * path:architecture/api-allocator
 * ## mapi_allocator_uni
 * ### Prototype
 * ```c
 * void *mapi_allocator_uni(morphine_instance_t I, void *p, size_t size)
 * ```
 * ### Parameters
 * * `I` - instance
 * * `p` - pointer (may be null)
 * * `size` - size of required memory
 * > [!WARNING]
 * > The pointer can only be the one that was produced `mapi_allocator_uni` or `mapi_allocator_vec`
 * ### Result
 * Pointer to allocated memory or null
 * ### Description
 * * If the pointer is null and the size is greater than 0, then allocates new memory chunk and returns it **_(side effect: gc step)_**
 * * If the pointer isn't null, but the size is greater than 0, then it will resize the allocated memory chunk and return a new pointer to it **_(side effect: gc step if the size of the new memory chunk will increase)_**
 * * If size is 0 then free the pointer (if the pointer is null, then does nothing) and return null
 * {{end}}
 */
MORPHINE_API void *mapi_allocator_uni(morphine_instance_t I, void *p, size_t size) {
    return allocI_uni(I, p, size);
}

/*
 * {{docs body}}
 * path:architecture/api-allocator
 * ## mapi_allocator_vec
 * ### Prototype
 * ```c
 * void *mapi_allocator_vec(morphine_instance_t I, void *p, size_t n, size_t size)
 * ```
 * ### Parameters
 * * `I` - instance
 * * `p` - pointer (may be null)
 * * `n` - size multiplier
 * * `size` - size of required memory
 * > [!WARNING]
 * > The pointer can only be the one that was produced `mapi_allocator_uni` or `mapi_allocator_vec`
 * ### Result
 * Pointer to allocated memory or null
 * ### Description
 * Same behavior as `mapi_allocator_uni`, but the size is safely multiplied by a multiplier
 * {{end}}
 */
MORPHINE_API void *mapi_allocator_vec(morphine_instance_t I, void *p, size_t n, size_t size) {
    return allocI_vec(I, p, n, size);
}

/*
 * {{docs body}}
 * path:architecture/api-allocator
 * ## mapi_allocator_free
 * ### Prototype
 * ```c
 * void mapi_allocator_free(morphine_instance_t I, void *p)
 * ```
 * ### Parameters
 * * `I` - instance
 * * `p` - pointer (may be null)
 * > [!WARNING]
 * > The pointer can only be the one that was produced `mapi_allocator_uni` or `mapi_allocator_vec`
 * ### Description
 * Frees the pointer (if the pointer is null, then does nothing)
 * {{end}}
 */
MORPHINE_API void mapi_allocator_free(morphine_instance_t I, void *p) {
    allocI_free(I, p);
}
