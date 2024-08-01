//
// Created by why-iskra on 02.04.2024.
//

#include "morphine/object/iterator.h"
#include "morphine/core/throw.h"
#include "morphine/object/table.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/object/vector.h"
#include "morphine/gc/safe.h"

struct iterator *iteratorI_create(morphine_instance_t I, struct value value) {
    struct table *table = valueI_safe_as_table(value, NULL);
    struct vector *vector = valueI_safe_as_vector(value, NULL);
    if (table == NULL && vector == NULL) {
        throwI_error(I, "iterator only supports table");
    }

    struct iterator *result = allocI_uni(I, NULL, sizeof(struct iterator));

    (*result) = (struct iterator) {
        .name.key = valueI_nil,
        .name.value = valueI_nil,
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

void iteratorI_init(
    morphine_instance_t I,
    struct iterator *iterator,
    struct value key_name,
    struct value value_name
) {
    if (iterator == NULL) {
        throwI_error(I, "iterator is null");
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
            throwI_panic(I, "unknown iterable type");
    }

    iterator->name.key = key_name;
    iterator->name.value = value_name;

    gcI_barrier(I, iterator, iterator->next.key);
    gcI_barrier(I, iterator, key_name);
    gcI_barrier(I, iterator, value_name);
}

bool iteratorI_has(morphine_instance_t I, struct iterator *iterator) {
    if (iterator == NULL) {
        throwI_error(I, "iterator is null");
    }

    return iterator->next.has;
}

struct pair iteratorI_next(morphine_instance_t I, struct iterator *iterator) {
    if (iterator == NULL) {
        throwI_error(I, "iterator is null");
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
            throwI_panic(I, "unknown iterable type");
    }

    gcI_barrier(I, iterator, iterator->next.key);

    return result;
}

struct table *iteratorI_next_table(morphine_instance_t I, struct iterator *iterator) {
    struct pair pair = iteratorI_next(I, iterator);

    struct table *table = tableI_create(I);
    size_t rollback = gcI_safe_obj(I, objectI_cast(table));

    tableI_set(I, table, iterator->name.key, pair.key);
    tableI_set(I, table, iterator->name.value, pair.value);

    gcI_reset_safe(I, rollback);

    return table;
}
