//
// Created by why-iskra on 16.11.2024.
//

#pragma once

#include "morphine/core/throw.h"
#include "morphine/object/string.h"
#include "morphine/platform/convert.h"

static inline struct string *convertI_to_string(morphine_instance_t I, struct value value) {
    switch (value.type) {
        case VALUE_TYPE_NIL:
            return stringI_create(I, "nil");
        case VALUE_TYPE_INTEGER:
            return stringI_createf(I, "%"MLIMIT_INTEGER_PR, value.integer);
        case VALUE_TYPE_DECIMAL:
            return stringI_createf(I, "%"MLIMIT_DECIMAL_PR, value.decimal);
        case VALUE_TYPE_BOOLEAN:
            return stringI_create(I, value.boolean ? "true" : "false");
        case VALUE_TYPE_STRING:
            return valueI_as_string(value);
        case VALUE_TYPE_USERDATA:
            return stringI_createf(I, "[object:userdata:%"PRIxPTR"]", (uintptr_t) value.object.userdata);
        case VALUE_TYPE_TABLE:
            return stringI_createf(I, "[object:table:%"PRIxPTR"]", (uintptr_t) value.object.table);
        case VALUE_TYPE_VECTOR:
            return stringI_createf(I, "[object:vector:%"PRIxPTR"]", (uintptr_t) value.object.vector);
        case VALUE_TYPE_CLOSURE:
            return stringI_createf(I, "[object:closure:%"PRIxPTR"]", (uintptr_t) value.object.closure);
        case VALUE_TYPE_COROUTINE:
            return stringI_createf(I, "[object:coroutine:%"PRIxPTR"]", (uintptr_t) value.object.coroutine);
        case VALUE_TYPE_REFERENCE:
            return stringI_createf(I, "[object:reference:%"PRIxPTR"]", (uintptr_t) value.object.reference);
        case VALUE_TYPE_EXCEPTION:
            return stringI_createf(I, "[object:exception:%"PRIxPTR"]", (uintptr_t) value.object.exception);
        case VALUE_TYPE_FUNCTION:
            return stringI_createf(I, "[object:function:%"PRIxPTR"]", (uintptr_t) value.object.function);
        case VALUE_TYPE_NATIVE:
            return stringI_createf(I, "[object:native:%"PRIxPTR"]", (uintptr_t) value.object.native);
        case VALUE_TYPE_STREAM:
            return stringI_createf(I, "[object:stream:%"PRIxPTR"]", (uintptr_t) value.object.stream);
        case VALUE_TYPE_RAW:
            return stringI_createf(I, "[raw:%"PRIxPTR"]", value.raw);
    }

    throwI_panic(I, "unsupported type");
}

static inline ml_integer convertI_to_integer(morphine_instance_t I, struct value value) {
    if (valueI_is_integer(value)) {
        return valueI_as_integer(value);
    } else if (valueI_is_decimal(value)) {
        return (ml_integer) valueI_as_decimal(value);
    } else if (valueI_is_boolean(value)) {
        if (valueI_as_boolean(value)) {
            return 1;
        } else {
            return 0;
        }
    }

    struct string *str = valueI_safe_as_string(value, NULL);
    if (str != NULL) {
        ml_integer integer;
        bool compatible = stringI_is_cstr(str);
        if (compatible && platformI_string2integer(str->chars, &integer, 10)) {
            return integer;
        }
    }

    throwI_errorf(I, "cannot convert %s to integer", valueI_type(I, value, false));
}

static inline ml_integer convertI_to_basedinteger(morphine_instance_t I, struct value value, ml_size base) {
    struct string *str = valueI_safe_as_string(value, NULL);
    if (str != NULL) {
        ml_integer integer;
        bool compatible = stringI_is_cstr(str);
        if (compatible && platformI_string2integer(str->chars, &integer, base)) {
            return integer;
        }
    }

    throwI_errorf(I, "cannot convert %s to based integer", valueI_type(I, value, false));
}

static inline ml_decimal convertI_to_decimal(morphine_instance_t I, struct value value) {
    if (valueI_is_integer(value)) {
        return (ml_decimal) valueI_as_integer(value);
    } else if (valueI_is_decimal(value)) {
        return valueI_as_decimal(value);
    } else if (valueI_is_boolean(value)) {
        if (valueI_as_boolean(value)) {
            return 1;
        } else {
            return 0;
        }
    }

    struct string *str = valueI_safe_as_string(value, NULL);
    if (str != NULL) {
        ml_decimal decimal;
        bool compatible = stringI_is_cstr(str);
        if (compatible && platformI_string2decimal(str->chars, &decimal)) {
            return decimal;
        }
    }

    throwI_errorf(I, "cannot convert %s to decimal", valueI_type(I, value, false));
}
