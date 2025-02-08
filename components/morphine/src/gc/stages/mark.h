//
// Created by why-iskra on 18.04.2024.
//

#pragma once

#include "morphine/core/throw.h"
#include "morphine/gc/pools.h"
#include "morphine/object/closure.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/exception.h"
#include "morphine/object/function.h"
#include "morphine/object/native.h"
#include "morphine/object/stream.h"
#include "morphine/object/string.h"
#include "morphine/object/table.h"
#include "morphine/object/userdata.h"
#include "morphine/object/vector.h"

static inline bool mark_object(morphine_instance_t I, struct object *object) {
    if (object->color == OBJ_COLOR_WHITE) {
        object->color = OBJ_COLOR_GREY;
        gcI_pools_remove(object, &I->G.pools.allocated);
        gcI_pools_insert(object, &I->G.pools.grey);

        return true;
    }

    return false;
}

static inline bool mark_value(morphine_instance_t I, struct value value) {
    if (valueI_is_object(value)) {
        return mark_object(I, valueI_as_object(value));
    }

    return false;
}

static inline size_t size_table(struct table *table) {
    return sizeof(struct table) + ((size_t) table->hashmap.buckets.count) * sizeof(struct bucket)
           + table->hashmap.hashing.size * sizeof(struct tree);
}

static inline size_t size_closure(mattr_unused struct closure *closure) {
    return sizeof(struct closure) + ((size_t) closure->size) * sizeof(struct value);
}

static inline size_t size_vector(struct vector *vector) {
    return sizeof(struct vector) + ((size_t) vector->size.real) * sizeof(struct value);
}

static inline size_t size_function(struct function *function) {
    return sizeof(struct function) + ((size_t) function->instructions_count) * sizeof(morphine_instruction_t)
           + ((size_t) function->constants_count) * sizeof(struct value);
}

static inline size_t size_userdata(mattr_unused struct userdata *userdata) {
    return sizeof(struct userdata);
}

static inline size_t size_coroutine(struct coroutine *coroutine) {
    return sizeof(struct coroutine) + ((size_t) coroutine->stack.size) * sizeof(struct value)
           + ((size_t) coroutine->callstack.size) * sizeof(struct callframe);
}

static inline size_t size_native(mattr_unused struct native *native) {
    return sizeof(struct native);
}

static inline size_t size_exception(struct exception *exception) {
    return sizeof(struct exception) + ((size_t) exception->stacktrace.size) * sizeof(struct stacktrace_element);
}

static inline size_t size_string(struct string *string) {
    return sizeof(struct string) + (((size_t) string->size) + 1) * sizeof(char);
}

static inline size_t size_stream(mattr_unused struct stream *stream) {
    return sizeof(struct stream);
}

static inline size_t mark_internal(morphine_instance_t I, struct object *obj) {
    switch (obj->type) {
        case OBJ_TYPE_TABLE: {
            struct table *table = cast(struct table *, obj);

            bool weak_key = false;
            bool weak_val = false;
            if (table->metatable != NULL) {
                mark_object(I, objectI_cast(table->metatable));

                struct value result;
                if (metatableI_test(I, valueI_object(table), MTYPE_METAFIELD_WEAK_KEY, &result)) {
                    weak_key = valueI_tobool(result);
                }
                if (metatableI_test(I, valueI_object(table), MTYPE_METAFIELD_WEAK_VALUE, &result)) {
                    weak_val = valueI_tobool(result);
                }
            }

            if (!weak_key || !weak_val) {
                struct bucket *current = table->hashmap.buckets.head;
                while (current != NULL) {
                    if (!weak_key) {
                        mark_value(I, current->pair.key);
                    }

                    if (!weak_val) {
                        mark_value(I, current->pair.value);
                    }

                    current = current->ll.prev;
                }
            }

            return size_table(table);
        }
        case OBJ_TYPE_VECTOR: {
            struct vector *vector = cast(struct vector *, obj);

            for (ml_size i = 0; i < vector->size.accessible; i++) {
                mark_value(I, vector->values[i]);
            }

            return size_vector(vector);
        }
        case OBJ_TYPE_CLOSURE: {
            struct closure *closure = cast(struct closure *, obj);

            mark_value(I, closure->callable);
            for (ml_size i = 0; i < closure->size; i++) {
                mark_value(I, closure->values[i]);
            }

            return size_closure(closure);
        }
        case OBJ_TYPE_FUNCTION: {
            struct function *function = cast(struct function *, obj);

            mark_object(I, objectI_cast(function->name));

            for (ml_size i = 0; i < function->constants_count; i++) {
                mark_value(I, function->constants[i]);
            }

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

            mark_object(I, objectI_cast(coroutine->name));
            mark_value(I, coroutine->env);

            if (coroutine->exception != NULL) {
                mark_object(I, objectI_cast(coroutine->exception));
            }

            mark_value(I, coroutine->callstack.result);

            for (ml_size i = 0; i < coroutine->stack.top; i++) {
                mark_value(I, coroutine->stack.allocated[i]);
            }

            return size_coroutine(coroutine);
        }
        case OBJ_TYPE_EXCEPTION: {
            struct exception *exception = cast(struct exception *, obj);

            mark_value(I, exception->value);

            if (exception->stacktrace.name != NULL) {
                mark_object(I, objectI_cast(exception->stacktrace.name));
            }

            for (ml_size i = 0; i < exception->stacktrace.size; i++) {
                struct string *name = exception->stacktrace.elements[i].name;
                if (name != NULL) {
                    mark_object(I, objectI_cast(name));
                }
            }

            return size_exception(exception);
        }
        case OBJ_TYPE_NATIVE: {
            struct native *native = cast(struct native *, obj);
            mark_object(I, objectI_cast(native->name));
            return size_native(native);
        }
        case OBJ_TYPE_STREAM: {
            struct stream *stream = cast(struct stream *, obj);
            mark_value(I, stream->hold);
            return size_stream(stream);
        }
        case OBJ_TYPE_STRING: {
            return size_string(cast(struct string *, obj));
        }
    }

    throwI_panic(I, "unsupported object");
}

static inline size_t size_obj(morphine_instance_t I, struct object *obj) {
    switch (obj->type) {
        case OBJ_TYPE_TABLE: {
            return size_table(cast(struct table *, obj));
        }
        case OBJ_TYPE_VECTOR: {
            return size_vector(cast(struct vector *, obj));
        }
        case OBJ_TYPE_CLOSURE: {
            return size_closure(cast(struct closure *, obj));
        }
        case OBJ_TYPE_FUNCTION: {
            return size_function(cast(struct function *, obj));
        }
        case OBJ_TYPE_USERDATA: {
            return size_userdata(cast(struct userdata *, obj));
        }
        case OBJ_TYPE_COROUTINE: {
            return size_coroutine(cast(struct coroutine *, obj));
        }
        case OBJ_TYPE_NATIVE: {
            return size_native(cast(struct native *, obj));
        }
        case OBJ_TYPE_EXCEPTION: {
            return size_exception(cast(struct exception *, obj));
        }
        case OBJ_TYPE_STRING: {
            return size_string(cast(struct string *, obj));
        }
        case OBJ_TYPE_STREAM: {
            return size_stream(cast(struct stream *, obj));
        }
    }

    throwI_panic(I, "unsupported object");
}
