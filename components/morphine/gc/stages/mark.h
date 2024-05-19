//
// Created by why-iskra on 18.04.2024.
//

#pragma once

#include "size.h"
#include "morphine/gc/pools.h"

static inline void mark_object(morphine_instance_t I, struct object *object) {
    if (object->color == OBJ_COLOR_WHITE) {
        object->color = OBJ_COLOR_GREY;
        gcI_pools_remove(object, &I->G.pools.allocated);
        gcI_pools_insert(object, &I->G.pools.grey);
    }
}

static inline void mark_value(morphine_instance_t I, struct value value) {
    if (valueI_is_object(value)) {
        mark_object(I, valueI_as_object(value));
    }
}

static inline size_t mark_internal(morphine_instance_t I, struct object *obj) {
    switch (obj->type) {
        case OBJ_TYPE_TABLE: {
            struct table *table = cast(struct table *, obj);

            if (table->metatable != NULL) {
                mark_object(I, objectI_cast(table->metatable));
            }

            struct bucket *current = table->hashmap.buckets.head;
            while (current != NULL) {
                mark_value(I, current->pair.key);
                mark_value(I, current->pair.value);

                current = current->ll.prev;
            }

            return size_table(table);
        }
        case OBJ_TYPE_VECTOR: {
            struct vector *vector = cast(struct vector *, obj);

            for (size_t i = 0; i < vector->size.accessible; i++) {
                mark_value(I, vector->values[i]);
            }

            return size_vector(vector);
        }
        case OBJ_TYPE_CLOSURE: {
            struct closure *closure = cast(struct closure *, obj);

            mark_value(I, closure->callable);
            for (size_t i = 0; i < closure->size; i++) {
                mark_value(I, closure->values[i]);
            }

            return size_closure(closure);
        }
        case OBJ_TYPE_FUNCTION: {
            struct function *function = cast(struct function *, obj);

            for (size_t i = 0; i < function->constants_count; i++) {
                mark_value(I, function->constants[i]);
            }

            for (size_t i = 0; i < function->statics_count; i++) {
                mark_value(I, function->statics[i]);
            }

            mark_value(I, function->registry_key);

            return size_function(function);
        }
        case OBJ_TYPE_USERDATA: {
            struct userdata *userdata = cast(struct userdata *, obj);

            if (userdata->metatable != NULL) {
                mark_object(I, objectI_cast(userdata->metatable));
            }

            return size_userdata(userdata);
        }
        case OBJ_TYPE_COROUTINE: {
            morphine_coroutine_t coroutine = cast(morphine_coroutine_t, obj);

            mark_value(I, coroutine->env);
            for (size_t i = 0; i < coroutine->stack.top; i++) {
                mark_value(I, coroutine->stack.allocated[i]);
            }

            return size_coroutine(coroutine);
        }
        case OBJ_TYPE_NATIVE: {
            struct native *native = cast(struct native *, obj);
            mark_value(I, native->registry_key);

            return size_native(native);
        }
        case OBJ_TYPE_ITERATOR: {
            struct iterator *iterator = cast(struct iterator *, obj);

            mark_object(I, objectI_cast(iterator->iterable.object));
            mark_value(I, iterator->next.key);

            return size_iterator(iterator);
        }
        case OBJ_TYPE_STRING: {
            return size_string(cast(struct string *, obj));
        }
        case OBJ_TYPE_REFERENCE: {
            return size_reference(cast(struct reference *, obj));
        }
        case OBJ_TYPE_SIO: {
            return size_sio(cast(struct sio *, obj));
        }
    }

    throwI_panic(I, "Unsupported object");
}
