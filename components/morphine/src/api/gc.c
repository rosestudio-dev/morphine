//
// Created by whyiskra on 28.12.23.
//

/*
 * {{docs header}}
 * path:architecture/api-garbage-collector
 * # Garbage collector API
 * ## Description
 * API for working with garbage collector
 * > See more [garbage collector](garbage-collector), [coroutines](coroutines)
 * {{end}}
 */

#include "morphine/api.h"
#include "morphine/object/coroutine.h"
#include "morphine/core/instance.h"
#include "morphine/gc/control.h"
#include "morphine/gc/status.h"

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_full
 * ### Prototype
 * ```c
 * void mapi_gc_full(morphine_instance_t I)
 * ```
 * ### Parameters
 * * `I` - instance
 * ### Description
 * Cancels current gc cycle and runs full collection
 * {{end}}
 */
MORPHINE_API void mapi_gc_full(morphine_instance_t I) {
    gcI_full(I, 0);
}

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_work
 * ### Prototype
 * ```c
 * void mapi_gc_work(morphine_instance_t I)
 * ```
 * ### Parameters
 * * `I` - instance
 * ### Description
 * Runs one gc step
 * {{end}}
 */
MORPHINE_API void mapi_gc_work(morphine_instance_t I) {
    gcI_work(I, 0);
}

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_force
 * ### Prototype
 * ```c
 * void mapi_gc_force(morphine_instance_t I)
 * ```
 * ### Parameters
 * * `I` - instance
 * ### Description
 * Runs a gc cycle
 * {{end}}
 */
MORPHINE_API void mapi_gc_force(morphine_instance_t I) {
    gcI_force(I);
}

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_is_running
 * ### Prototype
 * ```c
 * bool mapi_gc_is_running(morphine_instance_t I)
 * ```
 * ### Parameters
 * * `I` - instance
 * ### Result
 * GC is running
 * ### Description
 * Checks gc is running
 * {{end}}
 */
MORPHINE_API bool mapi_gc_is_running(morphine_instance_t I) {
    return gcI_isrunning(I);
}

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_enable
 * ### Prototype
 * ```c
 * void mapi_gc_enable(morphine_instance_t I)
 * ```
 * ### Parameters
 * * `I` - instance
 * ### Description
 * Enables gc
 * {{end}}
 */
MORPHINE_API void mapi_gc_enable(morphine_instance_t I) {
    gcI_enable(I);
}

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_disable
 * ### Prototype
 * ```c
 * void mapi_gc_disable(morphine_instance_t I)
 * ```
 * ### Parameters
 * * `I` - instance
 * ### Description
 * Disables gc
 * {{end}}
 */
MORPHINE_API void mapi_gc_disable(morphine_instance_t I) {
    gcI_disable(I);
}

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_is_enabled
 * ### Prototype
 * ```c
 * bool mapi_gc_is_enabled(morphine_instance_t I)
 * ```
 * ### Parameters
 * * `I` - instance
 * ### Result
 * GC is enabled
 * ### Description
 * Checks gc is enabled
 * {{end}}
 */
MORPHINE_API bool mapi_gc_is_enabled(morphine_instance_t I) {
    return gcI_isenabled(I);
}

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_status
 * ### Prototype
 * ```c
 * const char *mapi_gc_status(morphine_instance_t I)
 * ```
 * ### Parameters
 * * `I` - instance
 * ### Result
 * String of gc status
 * ### Description
 * Gets gc status
 * {{end}}
 */
MORPHINE_API const char *mapi_gc_status(morphine_instance_t I) {
    return gcI_status(I);
}

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_max_allocated
 * ### Prototype
 * ```c
 * size_t mapi_gc_max_allocated(morphine_instance_t I)
 * ```
 * ### Parameters
 * * `I` - instance
 * ### Result
 * Count of max allocated bytes
 * ### Description
 * Gets count of max allocated bytes
 * {{end}}
 */
MORPHINE_API size_t mapi_gc_max_allocated(morphine_instance_t I) {
    return I->G.bytes.max_allocated;
}

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_allocated
 * ### Prototype
 * ```c
 * size_t mapi_gc_allocated(morphine_instance_t I)
 * ```
 * ### Parameters
 * * `I` - instance
 * ### Result
 * Count of allocated bytes
 * ### Description
 * Gets count of allocated bytes
 * {{end}}
 */
MORPHINE_API size_t mapi_gc_allocated(morphine_instance_t I) {
    return I->G.bytes.allocated;
}

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_change_limit
 * ### Prototype
 * ```c
 * void mapi_gc_change_limit(morphine_instance_t I, size_t value)
 * ```
 * ### Parameters
 * * `I` - instance
 * * `value` - bytes
 * ### Description
 * Sets bytes into gc limit
 * {{end}}
 */
MORPHINE_API void mapi_gc_change_limit(morphine_instance_t I, size_t value) {
    gcI_change_limit(I, value);
}

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_change_threshold
 * ### Prototype
 * ```c
 * void mapi_gc_change_threshold(morphine_instance_t I, size_t value)
 * ```
 * ### Parameters
 * * `I` - instance
 * * `value` - bytes
 * ### Description
 * Sets bytes into gc threshold
 * {{end}}
 */
MORPHINE_API void mapi_gc_change_threshold(morphine_instance_t I, size_t value) {
    gcI_change_threshold(I, value);
}

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_change_grow
 * ### Prototype
 * ```c
 * void mapi_gc_change_grow(morphine_instance_t I, uint16_t value)
 * ```
 * ### Parameters
 * * `I` - instance
 * * `value` - grow factor (percentage)
 * ### Description
 * Sets percentage into gc grow factor
 * {{end}}
 */
MORPHINE_API void mapi_gc_change_grow(morphine_instance_t I, uint16_t value) {
    gcI_change_grow(I, value);
}

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_change_deal
 * ### Prototype
 * ```c
 * void mapi_gc_change_deal(morphine_instance_t I, uint16_t value)
 * ```
 * ### Parameters
 * * `I` - instance
 * * `value` - deal factor (percentage)
 * ### Description
 * Sets percentage into gc deal factor
 * {{end}}
 */
MORPHINE_API void mapi_gc_change_deal(morphine_instance_t I, uint16_t value) {
    gcI_change_deal(I, value);
}

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_change_pause
 * ### Prototype
 * ```c
 * void mapi_gc_change_pause(morphine_instance_t I, uint8_t value)
 * ```
 * ### Parameters
 * * `I` - instance
 * * `value` - 2^value bytes
 * ### Description
 * Sets bytes into gc pause
 * {{end}}
 */
MORPHINE_API void mapi_gc_change_pause(morphine_instance_t I, uint8_t value) {
    gcI_change_pause(I, value);
}

/*
 * {{docs body}}
 * path:architecture/api-garbage-collector
 * ## mapi_gc_change_cache_callinfo
 * ### Prototype
 * ```c
 * void mapi_gc_change_cache_callinfo(morphine_instance_t I, size_t value)
 * ```
 * ### Parameters
 * * `I` - instance
 * * `value` - count of callinfo
 * ### Description
 * Sets count of callinfo for cache into gc
 * {{end}}
 */
MORPHINE_API void mapi_gc_change_cache_callinfo(morphine_instance_t I, size_t value) {
    gcI_change_cache_callinfo(I, value);
}
