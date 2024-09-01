//
// Created by whyiskra on 16.12.23.
//

#include <string.h>
#include "morphine/core/value.h"
#include "morphine/core/throw.h"
#include "morphine/core/usertype.h"
#include "morphine/object/string.h"
#include "morphine/object/function.h"
#include "morphine/object/native.h"
#include "morphine/object/coroutine.h"
#include "morphine/object/userdata.h"
#include "morphine/platform/conversions.h"

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
        case VALUE_TYPE_RAW:
            return a.raw == b.raw;
        case VALUE_TYPE_STRING: {
            struct string *str_a = valueI_as_string(a);
            struct string *str_b = valueI_as_string(b);
            bool equal_size = str_a->size == str_b->size;

            return equal_size && (memcmp(str_a->chars, str_b->chars, sizeof(char) * str_a->size) == 0);
        }
        case VALUE_TYPE_USERDATA:
        case VALUE_TYPE_TABLE:
        case VALUE_TYPE_VECTOR:
        case VALUE_TYPE_CLOSURE:
        case VALUE_TYPE_COROUTINE:
        case VALUE_TYPE_FUNCTION:
        case VALUE_TYPE_NATIVE:
        case VALUE_TYPE_REFERENCE:
        case VALUE_TYPE_SIO:
        case VALUE_TYPE_ITERATOR:
            return a.object.header == b.object.header;
    }

    throwI_panic(I, "unsupported type");
}

struct value valueI_value2string(morphine_instance_t I, struct value value) {
    switch (value.type) {
        case VALUE_TYPE_NIL:
            return valueI_object(stringI_create(I, "nil"));
        case VALUE_TYPE_INTEGER:
            return valueI_object(stringI_createf(I, "%"MLIMIT_INTEGER_PR, value.integer));
        case VALUE_TYPE_DECIMAL:
            return valueI_object(stringI_createf(I, "%"MLIMIT_DECIMAL_PR, value.decimal));
        case VALUE_TYPE_BOOLEAN:
            return valueI_object(stringI_create(I, value.boolean ? "true" : "false"));
        case VALUE_TYPE_STRING:
            return value;
        case VALUE_TYPE_USERDATA:
            if (valueI_as_userdata(value)->is_typed) {
                struct usertype_info info = usertypeI_info(I, valueI_as_userdata(value)->typed.usertype);
                return valueI_object(stringI_createf(I, "[object:userdata:%p|%s]", value.object.userdata, info.name));
            } else {
                return valueI_object(stringI_createf(I, "[object:userdata:%p]", value.object.userdata));
            }
        case VALUE_TYPE_TABLE:
            return valueI_object(stringI_createf(I, "[object:table:%p]", value.object.table));
        case VALUE_TYPE_VECTOR:
            return valueI_object(stringI_createf(I, "[object:vector:%p]", value.object.vector));
        case VALUE_TYPE_ITERATOR:
            return valueI_object(stringI_createf(I, "[object:iterator:%p]", value.object.iterator));
        case VALUE_TYPE_CLOSURE:
            return valueI_object(stringI_createf(I, "[object:closure:%p]", value.object.closure));
        case VALUE_TYPE_COROUTINE:
            return valueI_object(stringI_createf(
                I,
                "[object:coroutine:%p|%s]",
                value.object.coroutine,
                valueI_as_coroutine(value)->name
            ));
        case VALUE_TYPE_REFERENCE:
            return valueI_object(stringI_createf(I, "[object:reference:%p]", value.object.reference));
        case VALUE_TYPE_FUNCTION:
            return valueI_object(stringI_createf(
                I,
                "[object:function:%p|%s]",
                value.object.function,
                valueI_as_function(value)->name
            ));
        case VALUE_TYPE_NATIVE:
            return valueI_object(stringI_createf(
                I,
                "[object:native:%p|%s]",
                value.object.native,
                valueI_as_native(value)->name
            ));
        case VALUE_TYPE_SIO:
            return valueI_object(stringI_createf(I, "[object:sio:%p]", value.object.native));
        case VALUE_TYPE_RAW:
            return valueI_object(stringI_createf(I, "[raw:%p]", value.raw));
    }

    throwI_panic(I, "unsupported type");
}

struct value valueI_value2integer(morphine_instance_t I, struct value value) {
    if (valueI_is_integer(value)) {
        return value;
    } else if (valueI_is_decimal(value)) {
        return valueI_integer((ml_integer) valueI_as_decimal(value));
    } else if (valueI_is_boolean(value)) {
        if (valueI_as_boolean(value)) {
            return valueI_integer(1);
        } else {
            return valueI_integer(0);
        }
    }

    struct string *str = valueI_safe_as_string(value, NULL);
    if (str != NULL) {
        ml_integer integer;
        if (platformI_string2integer(str->chars, &integer, 10)) {
            return valueI_integer(integer);
        } else {
            throwI_errorf(I, "cannot convert string '%s' to integer", str->chars);
        }
    }

    throwI_errorf(I, "cannot convert %s to integer", valueI_type(I, value, false));
}

struct value valueI_value2size(morphine_instance_t I, struct value value, const char *name) {
    if (name == NULL) {
        name = "size";
    }

    if (valueI_is_integer(value)) {
        return valueI_size(valueI_integer2namedsize(I, valueI_as_integer(value), name));
    } else if (valueI_is_decimal(value)) {
        return valueI_size(valueI_integer2namedsize(I, (ml_integer) valueI_as_decimal(value), name));
    } else if (valueI_is_boolean(value)) {
        if (valueI_as_boolean(value)) {
            return valueI_size(1);
        } else {
            return valueI_size(0);
        }
    }

    struct string *str = valueI_safe_as_string(value, NULL);
    if (str != NULL) {
        ml_size size;
        if (platformI_string2size(str->chars, &size, 10)) {
            return valueI_size(size);
        } else {
            throwI_errorf(I, "cannot convert string '%s' to %s", str->chars, name);
        }
    }

    throwI_errorf(I, "cannot convert %s to %s", valueI_type(I, value, false), name);
}

struct value valueI_value2basedinteger(morphine_instance_t I, struct value value, ml_size base) {
    struct string *str = valueI_safe_as_string(value, NULL);
    if (str != NULL) {
        bool check_size = strlen(str->chars) == str->size;

        ml_integer integer;
        if (check_size && platformI_string2integer(str->chars, &integer, base)) {
            return valueI_integer(integer);
        } else {
            throwI_errorf(
                I, "cannot convert string '%s' to integer with base %"MLIMIT_INTEGER_PR, str->chars, base
            );
        }
    }

    throwI_errorf(I, "cannot convert %s to based integer", valueI_type(I, value, false));
}

struct value valueI_value2basedsize(
    morphine_instance_t I,
    struct value value,
    ml_size base,
    const char *name
) {
    if (name == NULL) {
        name = "size";
    }

    struct string *str = valueI_safe_as_string(value, NULL);
    if (str != NULL) {
        bool check_size = strlen(str->chars) == str->size;

        ml_size size;
        if (check_size && platformI_string2size(str->chars, &size, base)) {
            return valueI_size(size);
        } else {
            throwI_errorf(
                I, "cannot convert string '%s' to %s with base %"MLIMIT_INTEGER_PR, str->chars, name, base
            );
        }
    }

    throwI_errorf(I, "cannot convert %s to based %s", valueI_type(I, value, false), name);
}

struct value valueI_value2decimal(morphine_instance_t I, struct value value) {
    if (valueI_is_integer(value)) {
        return valueI_decimal((ml_decimal) valueI_as_integer(value));
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
        ml_decimal decimal;
        if (platformI_string2decimal(str->chars, &decimal)) {
            return valueI_decimal(decimal);
        } else {
            throwI_errorf(I, "cannot convert string '%s' to decimal", str->chars);
        }
    }

    throwI_errorf(I, "cannot convert %s to decimal", valueI_type(I, value, false));
}

struct value valueI_value2boolean(morphine_instance_t I, struct value value) {
    if (valueI_is_integer(value)) {
        return valueI_boolean(valueI_as_integer(value) != 0);
    } else if (valueI_is_decimal(value)) {
        return valueI_boolean(valueI_as_decimal(value) != 0);
    } else if (valueI_is_boolean(value)) {
        return value;
    }

    struct string *str = valueI_safe_as_string(value, NULL);
    if (str != NULL) {
        if (str->size == 4 && memcmp(str->chars, "true", sizeof(char) * 4) == 0) {
            return valueI_boolean(true);
        } else if (str->size == 5 && memcmp(str->chars, "false", sizeof(char) * 5) == 0) {
            return valueI_boolean(false);
        } else {
            throwI_errorf(I, "cannot convert string '%s' to boolean", str->chars);
        }
    }

    throwI_errorf(I, "cannot convert %s to boolean", valueI_type(I, value, false));
}

static const char *type2string(morphine_instance_t I, enum value_type type) {
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
        case VALUE_TYPE_VECTOR:
            return "vector";
        case VALUE_TYPE_CLOSURE:
            return "closure";
        case VALUE_TYPE_COROUTINE:
            return "coroutine";
        case VALUE_TYPE_FUNCTION:
            return "function";
        case VALUE_TYPE_REFERENCE:
            return "reference";
        case VALUE_TYPE_ITERATOR:
            return "iterator";
        case VALUE_TYPE_SIO:
            return "sio";
        case VALUE_TYPE_RAW:
            return "raw";
    }

    throwI_panic(I, "unsupported type");
}


const char *valueI_type(morphine_instance_t I, struct value value, bool raw) {
    if (!raw) {
        struct userdata *userdata = valueI_safe_as_userdata(value, NULL);

        if (userdata != NULL && userdata->is_typed) {
            struct usertype_info info = usertypeI_info(I, userdata->typed.usertype);

            return info.name;
        }
    }

    return type2string(I, value.type);
}

bool valueI_is_type(morphine_instance_t I, const char *name, bool raw) {
    if (!raw && usertypeI_is_declared(I, name)) {
        return true;
    }

    for (enum value_type t = VALUE_TYPES_START; t < VALUE_TYPES_COUNT; t++) {
        if (strcmp(type2string(I, t), name) == 0) {
            return true;
        }
    }

    return false;
}

enum value_type valueI_string2type(morphine_instance_t I, const char *name) {
    for (enum value_type t = VALUE_TYPES_START; t < VALUE_TYPES_COUNT; t++) {
        if (strcmp(type2string(I, t), name) == 0) {
            return t;
        }
    }

    throwI_errorf(I, "unknown type '%s'", name);
}
