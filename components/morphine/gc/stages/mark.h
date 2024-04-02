//
// Created by whyiskra on 3/23/24.
//

#pragma once

#include "morphine/object/state.h"
#include "morphine/object/table.h"
#include "morphine/object/closure.h"
#include "morphine/object/userdata.h"
#include "morphine/object/proto.h"
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

            return tableI_allocated_size(table);
        }
        case OBJ_TYPE_CLOSURE: {
            struct closure *closure = cast(struct closure *, obj);

            mark_value(closure->callable);
            for (size_t i = 0; i < closure->size; i++) {
                mark_value(closure->values[i]);
            }

            return closureI_allocated_size(closure);
        }
        case OBJ_TYPE_PROTO: {
            struct proto *proto = cast(struct proto *, obj);

            for (size_t i = 0; i < proto->constants_count; i++) {
                mark_value(proto->constants[i]);
            }

            for (size_t i = 0; i < proto->statics_count; i++) {
                mark_value(proto->statics[i]);
            }

            mark_value(proto->registry_key);

            return protoI_allocated_size(proto);
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

            return userdataI_allocated_size(userdata);
        }
        case OBJ_TYPE_STATE: {
            morphine_state_t state = cast(morphine_state_t, obj);

            for (size_t i = 0; i < state->stack.top; i++) {
                mark_value(state->stack.allocated[i]);
            }

            return stateI_allocated_size(state);
        }
        case OBJ_TYPE_NATIVE: {
            struct native *native = cast(struct native *, obj);
            mark_value(native->registry_key);

            return nativeI_allocated_size();
        }
        case OBJ_TYPE_STRING: {
            struct string *string = cast(struct string *, obj);
            return stringI_allocated_size(string);
        }
        case OBJ_TYPE_REFERENCE: {
            return referenceI_allocated_size();
        }
        case OBJ_TYPE_ITERATOR: {
            struct iterator *iterator = cast(struct iterator *, obj);

            mark_object(objectI_cast(iterator->iterable));
            mark_value(iterator->next.key);

            return iteratorI_allocated_size();
        }
    }

    throwI_message_panic(I, NULL, "Unsupported object");
}
