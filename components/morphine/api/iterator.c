//
// Created by why-iskra on 02.04.2024.
//

#include "morphine/api.h"
#include "morphine/core/throw.h"
#include "morphine/stack/access.h"
#include "morphine/object/iterator.h"
#include "morphine/object/coroutine.h"

MORPHINE_API void mapi_iterator(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct iterator *iterator = iteratorI_create(U->I, value);

    stackI_push(U, valueI_object(iterator));
}

MORPHINE_API void mapi_iterator_init(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct iterator *iterator = valueI_as_iterator_or_error(U->I, value);

    iteratorI_init(U->I, iterator);
}

MORPHINE_API bool mapi_iterator_has(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct iterator *iterator = valueI_as_iterator_or_error(U->I, value);

    return iteratorI_has(U->I, iterator);
}

MORPHINE_API void mapi_iterator_next(morphine_coroutine_t U) {
    struct value value = stackI_peek(U, 0);
    struct iterator *iterator = valueI_as_iterator_or_error(U->I, value);

    struct pair pair = iteratorI_next(U->I, iterator);

    stackI_push(U, pair.key);
    stackI_push(U, pair.value);
}
