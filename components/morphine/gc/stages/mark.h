//
// Created by whyiskra on 3/23/24.
//

#pragma once

#include "morphine/object/coroutine.h"
#include "morphine/object/table.h"
#include "morphine/object/closure.h"
#include "morphine/object/userdata.h"
#include "morphine/object/function.h"
#include "morphine/object/reference.h"
#include "morphine/object/string.h"
#include "morphine/object/native.h"
#include "morphine/core/throw.h"
#include "morphine/object/iterator.h"
#include "size.h"

static inline bool mark_unmarked_object(struct object *object) {
    if (object->flags.mark) {
        return false;
    }

    object->flags.mark = true;
    return true;
}

static inline void mark_object(struct object *object) {
    object->flags.mark = true;
}

static inline void mark_value(struct value value) {
    if (valueI_is_object(value)) {
        mark_object(valueI_as_object(value));
    }
}

static inline size_t mark_internal(morphine_instance_t I, struct object *obj) {
    switch (obj->type) {
        case OBJ_TYPE_TABLE: {
            struct table *table = cast(struct table *, obj);

            if (table->metatable != NULL) {
                mark_object(objectI_cast(table->metatable));
            }

            struct bucket *current = table->hashmap.buckets.head;
            while (current != NULL) {
                mark_value(current->pair.key);
                mark_value(current->pair.value);

                current = current->ll.prev;
            }

            return size_table(table);
        }
        case OBJ_TYPE_CLOSURE: {
            struct closure *closure = cast(struct closure *, obj);

            mark_value(closure->callable);
            for (size_t i = 0; i < closure->size; i++) {
                mark_value(closure->values[i]);
            }

            return size_closure(closure);
        }
        case OBJ_TYPE_FUNCTION: {
            struct function *function = cast(struct function *, obj);

            for (size_t i = 0; i < function->constants_count; i++) {
                mark_value(function->constants[i]);
            }

            for (size_t i = 0; i < function->statics_count; i++) {
                mark_value(function->statics[i]);
            }

            mark_value(function->registry_key);

            return size_function(function);
        }
        case OBJ_TYPE_USERDATA: {
            struct userdata *userdata = cast(struct userdata *, obj);

            if (userdata->metatable != NULL) {
                mark_object(objectI_cast(userdata->metatable));
            }

            struct link *current = userdata->links.pool;
            while (current != NULL) {
                mark_object(objectI_cast(current->userdata));
                current = current->prev;
            }

            if (userdata->mark != NULL) {
                userdata->mark(I, userdata->data);
            }

            return size_userdata(userdata);
        }
        case OBJ_TYPE_COROUTINE: {
            morphine_coroutine_t coroutine = cast(morphine_coroutine_t, obj);

            mark_value(coroutine->env);
            for (size_t i = 0; i < coroutine->stack.top; i++) {
                mark_value(coroutine->stack.allocated[i]);
            }

            return size_coroutine(coroutine);
        }
        case OBJ_TYPE_NATIVE: {
            struct native *native = cast(struct native *, obj);
            mark_value(native->registry_key);

            return size_native(native);
        }
        case OBJ_TYPE_ITERATOR: {
            struct iterator *iterator = cast(struct iterator *, obj);

            mark_object(objectI_cast(iterator->iterable));
            mark_value(iterator->next.key);

            return size_iterator(iterator);
        }
        case OBJ_TYPE_STRING: {
            return size_string(cast(struct string *, obj));
        }
        case OBJ_TYPE_REFERENCE: {
            return size_reference(cast(struct reference *, obj));
        }
    }

    throwI_panic(I, "Unsupported object");
}
