//
// Created by why-iskra on 02.04.2024.
//

#include "morphine/object/iterator.h"
#include "morphine/core/throw.h"
#include "morphine/object/table.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/object/vector.h"

struct iterator *iteratorI_create(morphine_instance_t I, struct value value) {
    struct table *table = valueI_safe_as_table(value, NULL);
    struct vector *vector = valueI_safe_as_vector(value, NULL);
    if (table == NULL && vector == NULL) {
        throwI_error(I, "Iterator only supports table");
    }

    struct iterator *result = allocI_uni(I, NULL, sizeof(struct iterator));

    (*result) = (struct iterator) {
        .next.has = false,
        .next.key = valueI_nil,
    };

    if (table != NULL) {
        result->type = ITERATOR_TYPE_TABLE;
        result->iterable.table = table;
    } else {
        result->type = ITERATOR_TYPE_VECTOR;
        result->iterable.vector = vector;
    }

    objectI_init(I, objectI_cast(result), OBJ_TYPE_ITERATOR);

    return result;
}

void iteratorI_free(morphine_instance_t I, struct iterator *iterator) {
    allocI_free(I, iterator);
}

void iteratorI_init(morphine_instance_t I, struct iterator *iterator) {
    if (iterator == NULL) {
        throwI_error(I, "Iterator is null");
    }

    switch (iterator->type) {
        case ITERATOR_TYPE_TABLE:
            iterator->next.key = tableI_first(
                I, iterator->iterable.table,
                &iterator->next.has
            );
            break;
        case ITERATOR_TYPE_VECTOR:
            iterator->next.key = vectorI_first(
                I, iterator->iterable.vector,
                &iterator->next.has
            );
            break;
        default:
            throwI_panic(I, "Unknown iterable type");
    }

    gcI_barrier(I, iterator, iterator->next.key);
}

bool iteratorI_has(morphine_instance_t I, struct iterator *iterator) {
    if (iterator == NULL) {
        throwI_error(I, "Iterator is null");
    }

    return iterator->next.has;
}

struct pair iteratorI_next(morphine_instance_t I, struct iterator *iterator) {
    if (iterator == NULL) {
        throwI_error(I, "Iterator is null");
    }

    struct pair result;
    switch (iterator->type) {
        case ITERATOR_TYPE_TABLE:
            result = tableI_next(
                I, iterator->iterable.table,
                &iterator->next.key,
                &iterator->next.has
            );
            break;
        case ITERATOR_TYPE_VECTOR:
            result = vectorI_next(
                I, iterator->iterable.vector,
                &iterator->next.key,
                &iterator->next.has
            );
            break;
        default:
            throwI_panic(I, "Unknown iterable type");
    }

    gcI_barrier(I, iterator, iterator->next.key);

    return result;
}
