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

            return sizeof(struct table) +
                   table->hashmap.buckets.count * sizeof(struct bucket) +
                   table->hashmap.hashing.size * sizeof(struct bucket *);
        }
        case OBJ_TYPE_CLOSURE: {
            struct closure *closure = cast(struct closure *, obj);

            mark_value(closure->callable);
            for (size_t i = 0; i < closure->size; i++) {
                mark_value(closure->values[i]);
            }

            return sizeof(struct closure) +
                   closure->size * sizeof(struct value);
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

            return sizeof(struct function) +
                   (function->name_len + 1) * sizeof(char) +
                   function->instructions_count * sizeof(instruction_t) +
                   function->statics_count * sizeof(struct value) +
                   function->constants_count * sizeof(struct value);
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

            return sizeof(struct userdata) +
                   (userdata->name_len + 1) * sizeof(char) +
                   userdata->links.size * sizeof(struct link);
        }
        case OBJ_TYPE_COROUTINE: {
            morphine_coroutine_t coroutine = cast(morphine_coroutine_t, obj);

            mark_value(coroutine->env);
            for (size_t i = 0; i < coroutine->stack.top; i++) {
                mark_value(coroutine->stack.allocated[i]);
            }

            return sizeof(struct coroutine) +
                   coroutine->stack.size * sizeof(struct value) +
                   coroutine->callstack.size * sizeof(struct callstack);
        }
        case OBJ_TYPE_NATIVE: {
            struct native *native = cast(struct native *, obj);
            mark_value(native->registry_key);

            return sizeof(struct native) +
                   (native->name_len + 1) * sizeof(char);
        }
        case OBJ_TYPE_ITERATOR: {
            struct iterator *iterator = cast(struct iterator *, obj);

            mark_object(objectI_cast(iterator->iterable));
            mark_value(iterator->next.key);

            return sizeof(struct iterator);
        }
        case OBJ_TYPE_STRING: {
            struct string *string = cast(struct string *, obj);
            return sizeof(struct string) + (string->size + 1) * sizeof(char);
        }
        case OBJ_TYPE_REFERENCE: {
            return sizeof(struct reference);
        }
    }

    throwI_panic(I, "Unsupported object");
}
