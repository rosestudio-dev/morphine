//
// Created by why-iskra on 02.04.2024.
//

/*
 * {{docs header}}
 * path:vm/api-iterator
 * # Iterator API
 * ## Description
 * API for working with iterator
 * > See more [values and objects](values-and-objects)
 * {{end}}
 */

#include "morphine/api.h"
#include "morphine/core/throw.h"
#include "morphine/object/iterator.h"
#include "morphine/object/coroutine.h"

/*
 * {{docs body}}
 * path:vm/api-iterator
 * ## mapi_iterator
 * ### Prototype
 * ```c
 * void mapi_iterator(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Peeks the iterable value from the stack and push iterator value to the stack
 * {{end}}
 */
MORPHINE_API void mapi_iterator(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct iterator *iterator = iteratorI_create(U->I, value);

    stackI_push(U, valueI_object(iterator));
}

/*
 * {{docs body}}
 * path:vm/api-iterator
 * ## mapi_iterator_init
 * ### Prototype
 * ```c
 * void mapi_iterator_init(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Peeks the iterator value from the stack and initializes it
 * {{end}}
 */
MORPHINE_API void mapi_iterator_init(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct iterator *iterator = valueI_as_iterator_or_error(U->I, value);

    iteratorI_init(U->I, iterator, valueI_nil, valueI_nil);
}

/*
 * {{docs body}}
 * path:vm/api-iterator
 * ## mapi_iterator_has
 * ### Prototype
 * ```c
 * bool mapi_iterator_has(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Result
 * Has the next value
 * ### Description
 * Peeks the iterator value from the stack and checks for the next value of the iterator
 * {{end}}
 */
MORPHINE_API bool mapi_iterator_has(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct iterator *iterator = valueI_as_iterator_or_error(U->I, value);

    return iteratorI_has(U->I, iterator);
}

/*
 * {{docs body}}
 * path:vm/api-iterator
 * ## mapi_iterator_next
 * ### Prototype
 * ```c
 * bool mapi_iterator_next(morphine_coroutine_t U)
 * ```
 * ### Parameters
 * * `U` - coroutine
 * ### Description
 * Peeks the iterator value from the stack and pushes the pair of key-value onto the stack
 * {{end}}
 */
MORPHINE_API void mapi_iterator_next(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct iterator *iterator = valueI_as_iterator_or_error(U->I, value);

    struct pair pair = iteratorI_next(U->I, iterator);

    stackI_push(U, pair.key);
    stackI_push(U, pair.value);
}
