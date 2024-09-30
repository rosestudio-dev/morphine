//
// Created by why-iskra on 02.04.2024.
//

#include "morphine/object/iterator.h"
#include "morphine/object/table.h"
#include "morphine/object/vector.h"
#include "morphine/object/string.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/gc/barrier.h"
#include "morphine/gc/safe.h"

struct iterator *iteratorI_create(morphine_instance_t I, struct value value) {
    struct table *table = valueI_safe_as_table(value, NULL);
    struct vector *vector = valueI_safe_as_vector(value, NULL);
    struct string *string = valueI_safe_as_string(value, NULL);

    enum iterator_type type;
    struct object *object;
    if (table != NULL) {
        type = ITERATOR_TYPE_TABLE;
        object = objectI_cast(table);
    } else if (vector != NULL) {
        type = ITERATOR_TYPE_VECTOR;
        object = objectI_cast(vector);
    } else if (string != NULL) {
        type = ITERATOR_TYPE_STRING;
        object = objectI_cast(string);
    } else {
        throwI_error(I, "iterator only supports table, vector or string");
    }

    size_t rollback = gcI_safe_obj(I, object);

    // create
    struct iterator *result = allocI_uni(I, NULL, sizeof(struct iterator));
    (*result) = (struct iterator) {
        .type = type,
        .iterable.object = object,
        .name.key = valueI_nil,
        .name.value = valueI_nil,
        .next.has = false,
        .next.key = valueI_nil,
        .result.key = valueI_nil,
        .result.value = valueI_nil
    };

    objectI_init(I, objectI_cast(result), OBJ_TYPE_ITERATOR);

    gcI_reset_safe(I, rollback);

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
            iterator->next.key = tableI_iterator_first(
                I, iterator->iterable.table,
                &iterator->next.has
            );
            break;
        case ITERATOR_TYPE_VECTOR:
            iterator->next.key = vectorI_iterator_first(
                I, iterator->iterable.vector,
                &iterator->next.has
            );
            break;
        case ITERATOR_TYPE_STRING:
            iterator->next.key = stringI_iterator_first(
                I, iterator->iterable.string,
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

    switch (iterator->type) {
        case ITERATOR_TYPE_TABLE:
            iterator->result = tableI_iterator_next(
                I, iterator->iterable.table,
                &iterator->next.key,
                &iterator->next.has
            );
            break;
        case ITERATOR_TYPE_VECTOR:
            iterator->result = vectorI_iterator_next(
                I, iterator->iterable.vector,
                &iterator->next.key,
                &iterator->next.has
            );
            break;
        case ITERATOR_TYPE_STRING:
            iterator->result = stringI_iterator_next(
                I, iterator->iterable.string,
                &iterator->next.key,
                &iterator->next.has
            );
            break;
        default:
            throwI_panic(I, "unknown iterable type");
    }

    gcI_barrier(I, iterator, iterator->result.key);
    gcI_barrier(I, iterator, iterator->result.value);
    gcI_barrier(I, iterator, iterator->next.key);

    return iterator->result;
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
