//
// Created by whyiskra on 16.12.23.
//

#include <string.h>
#include "morphine/core/value.h"
#include "morphine/core/throw.h"
#include "morphine/object/string.h"
#include "morphine/object/proto.h"
#include "morphine/object/native.h"
#include "morphine/object/state.h"
#include "morphine/object/userdata.h"

bool valueI_equal(morphine_instance_t I, struct value a, struct value b) {
    if (likely(a.type != b.type)) {
        return false;
    }

    switch (a.type) {
        case VALUE_TYPE_NIL:
            return true;
        case VALUE_TYPE_INTEGER:
            return a.integer == b.integer;
        case VALUE_TYPE_DECIMAL:
            return a.decimal == b.decimal;
        case VALUE_TYPE_BOOLEAN:
            return a.boolean == b.boolean;
        case VALUE_TYPE_STRING:
            return strcmp(valueI_as_string(a)->chars, valueI_as_string(b)->chars) == 0;
        case VALUE_TYPE_USERDATA:
        case VALUE_TYPE_TABLE:
        case VALUE_TYPE_CLOSURE:
        case VALUE_TYPE_STATE:
        case VALUE_TYPE_PROTO:
        case VALUE_TYPE_NATIVE:
        case VALUE_TYPE_REFERENCE:
        case VALUE_TYPE_ITERATOR:
        case VALUE_TYPE_RAW:
            return a.object.header == b.object.header;
    }

    throwI_message_panic(I, NULL, "Unsupported type");
}

struct value valueI_value2string(morphine_instance_t I, struct value value) {
    switch (value.type) {
        case VALUE_TYPE_NIL:
            return valueI_object(stringI_create(I, "nil"));
        case VALUE_TYPE_INTEGER:
            return valueI_object(stringI_createf(I, "%"MORPHINE_INTEGER_PRINT, value.integer));
        case VALUE_TYPE_DECIMAL:
            return valueI_object(stringI_createf(I, "%"MORPHINE_DECIMAL_PRINT, value.decimal));
        case VALUE_TYPE_BOOLEAN:
            return valueI_object(stringI_create(I, value.boolean ? "true" : "false"));
        case VALUE_TYPE_STRING:
            return value;
        case VALUE_TYPE_USERDATA:
            return valueI_object(
                stringI_createf(I, "[object:userdata:%s:%p]", value.object.userdata->type, value.object.userdata)
            );
        case VALUE_TYPE_TABLE:
            return valueI_object(stringI_createf(I, "[object:table:%p]", value.object.table));
        case VALUE_TYPE_ITERATOR:
            return valueI_object(stringI_createf(I, "[object:iterator:%p]", value.object.iterator));
        case VALUE_TYPE_CLOSURE:
            return valueI_object(stringI_createf(I, "[object:closure:%p]", value.object.closure));
        case VALUE_TYPE_STATE:
            return valueI_object(stringI_createf(I, "[object:state:%p]", value.object.state));
        case VALUE_TYPE_REFERENCE:
            return valueI_object(stringI_createf(I, "[object:reference:%p]", value.object.reference));
        case VALUE_TYPE_PROTO:
            return valueI_object(stringI_createf(I, "[object:proto:%p|%s]", value.object.proto, valueI_as_proto(value)->name));
        case VALUE_TYPE_NATIVE:
            return valueI_object(stringI_createf(I, "[object:native:%p|%s]", value.object.native, valueI_as_native(value)->name));
        case VALUE_TYPE_RAW:
            return valueI_object(stringI_createf(I, "[raw:%p]", value.raw));
    }

    throwI_message_panic(I, NULL, "Unsupported type");
}

struct value valueI_value2integer(morphine_state_t S, struct value value) {
    if (valueI_is_integer(value)) {
        return value;
    } else if (valueI_is_decimal(value)) {
        return valueI_integer((morphine_integer_t) valueI_as_decimal(value));
    } else if (valueI_is_boolean(value)) {
        if (valueI_as_boolean(value)) {
            return valueI_integer(1);
        } else {
            return valueI_integer(0);
        }
    }

    struct string *str = valueI_safe_as_string(value, NULL);
    if (str != NULL) {
        morphine_integer_t integer;
        if (platformI_string2integer(str->chars, &integer, 10)) {
            return valueI_integer(integer);
        } else {
            throwI_errorf(S, "Cannot convert string '%s' to integer", str->chars);
        }
    }

    throwI_errorf(S, "Cannot convert %s to integer", valueI_type2string(S->I, value.type));
}

struct value valueI_value2decimal(morphine_state_t S, struct value value) {
    if (valueI_is_integer(value)) {
        return valueI_decimal((morphine_decimal_t) valueI_as_integer(value));
    } else if (valueI_is_decimal(value)) {
        return value;
    } else if (valueI_is_boolean(value)) {
        if (valueI_as_boolean(value)) {
            return valueI_decimal(1);
        } else {
            return valueI_decimal(0);
        }
    }

    struct string *str = valueI_safe_as_string(value, NULL);
    if (str != NULL) {
        morphine_decimal_t decimal;
        if (platformI_string2decimal(str->chars, &decimal)) {
            return valueI_decimal(decimal);
        } else {
            throwI_errorf(S, "Cannot convert string '%s' to decimal", str->chars);
        }
    }

    throwI_errorf(S, "Cannot convert %s to decimal", valueI_type2string(S->I, value.type));
}

struct value valueI_value2boolean(morphine_state_t S, struct value value) {
    if (valueI_is_integer(value)) {
        return valueI_boolean(valueI_as_integer(value) != 0);
    } else if (valueI_is_decimal(value)) {
        return valueI_boolean(valueI_as_decimal(value) != 0);
    } else if (valueI_is_boolean(value)) {
        return value;
    }

    struct string *str = valueI_safe_as_string(value, NULL);
    if (str != NULL) {
        if (strcmp(str->chars, "true") == 0) {
            return valueI_boolean(true);
        } else if (strcmp(str->chars, "false") == 0) {
            return valueI_boolean(false);
        } else {
            throwI_errorf(S, "Cannot convert string '%s' to boolean", str->chars);
        }
    }

    throwI_errorf(S, "Cannot convert %s to boolean", valueI_type2string(S->I, value.type));
}

const char *valueI_type2string(morphine_instance_t I, enum value_type type) {
    switch (type) {
        case VALUE_TYPE_NIL:
            return "nil";
        case VALUE_TYPE_INTEGER:
            return "integer";
        case VALUE_TYPE_DECIMAL:
            return "decimal";
        case VALUE_TYPE_BOOLEAN:
            return "boolean";
        case VALUE_TYPE_NATIVE:
            return "native";
        case VALUE_TYPE_STRING:
            return "string";
        case VALUE_TYPE_USERDATA:
            return "userdata";
        case VALUE_TYPE_TABLE:
            return "table";
        case VALUE_TYPE_CLOSURE:
            return "closure";
        case VALUE_TYPE_STATE:
            return "state";
        case VALUE_TYPE_PROTO:
            return "proto";
        case VALUE_TYPE_REFERENCE:
            return "reference";
        case VALUE_TYPE_ITERATOR:
            return "iterator";
        case VALUE_TYPE_RAW:
            return "raw";
    }

    throwI_message_panic(I, NULL, "Unsupported type");
}

enum value_type valueI_string2type(morphine_state_t S, const char *name) {
    for (enum value_type t = VALUE_TYPES_START; t < VALUE_TYPES_COUNT; t++) {
        if (strcmp(valueI_type2string(S->I, t), name) == 0) {
            return t;
        }
    }

    throwI_errorf(S, "Unknown type '%s'", name);
}
