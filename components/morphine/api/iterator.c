//
// Created by why-iskra on 02.04.2024.
//

#include "morphine/api.h"
#include "morphine/core/throw.h"
#include "morphine/stack/access.h"
#include "morphine/object/iterator.h"
#include "morphine/object/state.h"

MORPHINE_API void mapi_iterator(morphine_state_t S) {
    struct value value = stackI_peek(S, 0);
    struct iterator *iterator = iteratorI_create(S->I, value);

    stackI_push(S, valueI_object(iterator));
}

MORPHINE_API void mapi_iterator_init(morphine_state_t S) {
    struct value value = stackI_peek(S, 0);
    struct iterator *iterator = valueI_as_iterator_or_error(S, value);

    iteratorI_init(S->I, iterator);
}

MORPHINE_API bool mapi_iterator_has(morphine_state_t S) {
    struct value value = stackI_peek(S, 0);
    struct iterator *iterator = valueI_as_iterator_or_error(S, value);

    return iteratorI_has(S->I, iterator);
}

MORPHINE_API void mapi_iterator_next(morphine_state_t S) {
    struct value value = stackI_peek(S, 0);
    struct iterator *iterator = valueI_as_iterator_or_error(S, value);

    struct pair pair = iteratorI_next(S->I, iterator);

    stackI_push(S, pair.key);
    stackI_push(S, pair.value);
}
