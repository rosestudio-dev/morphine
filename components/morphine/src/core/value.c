//
// Created by whyiskra on 16.12.23.
//

#include "morphine/core/value.h"
#include "morphine/algorithm/hash.h"
#include "morphine/core/usertype.h"
#include "morphine/object/string.h"
#include "morphine/object/userdata.h"
#include "morphine/utils/compare.h"
#include <string.h>

#define declare_hash(t, n) static inline ml_hash n##2hash(t value) { return calchash((const uint8_t *) &value, sizeof(value)); }

declare_hash(void *, ptr);
declare_hash(ml_integer, integer);
declare_hash(ml_decimal, decimal);
declare_hash(bool, bool);
declare_hash(uintptr_t, uintptr);

int valueI_compare(struct value a, struct value b) {
    if (mm_likely(a.type != b.type)) {
        return smpcmp(a.type, b.type);
    }

    switch (a.type) {
        case VALUE_TYPE_NIL: return 0;
        case VALUE_TYPE_INTEGER: return smpcmp(a.integer, b.integer);
        case VALUE_TYPE_DECIMAL: return smpcmp(a.decimal, b.decimal);
        case VALUE_TYPE_BOOLEAN: return smpcmp(a.boolean, b.boolean);
        case VALUE_TYPE_RAW: return smpcmp(a.raw, b.raw);
        case VALUE_TYPE_STRING: return stringI_compare(valueI_as_string(a), valueI_as_string(b));
        case VALUE_TYPE_USERDATA:
        case VALUE_TYPE_TABLE:
        case VALUE_TYPE_VECTOR:
        case VALUE_TYPE_CLOSURE:
        case VALUE_TYPE_COROUTINE:
        case VALUE_TYPE_FUNCTION:
        case VALUE_TYPE_NATIVE:
        case VALUE_TYPE_EXCEPTION:
        case VALUE_TYPE_STREAM: return smpcmp(a.object.header, b.object.header);
    }
}

ml_hash valueI_hash(struct value value) {
    switch (value.type) {
        case VALUE_TYPE_NIL: return ptr2hash(valueI_as_nil(value));
        case VALUE_TYPE_INTEGER: return integer2hash(valueI_as_integer(value));
        case VALUE_TYPE_DECIMAL: return decimal2hash(valueI_as_decimal(value));
        case VALUE_TYPE_BOOLEAN: return bool2hash(valueI_as_boolean(value));
        case VALUE_TYPE_RAW: return uintptr2hash(valueI_as_raw(value));
        case VALUE_TYPE_STRING: return stringI_hash(valueI_as_string(value));
        case VALUE_TYPE_USERDATA:
        case VALUE_TYPE_TABLE:
        case VALUE_TYPE_VECTOR:
        case VALUE_TYPE_CLOSURE:
        case VALUE_TYPE_COROUTINE:
        case VALUE_TYPE_FUNCTION:
        case VALUE_TYPE_NATIVE:
        case VALUE_TYPE_EXCEPTION:
        case VALUE_TYPE_STREAM: return ptr2hash(valueI_as_object(value));
    }
}

const char *valueI_type(morphine_instance_t I, struct value value, bool raw) {
    if (!raw) {
        struct userdata *userdata = valueI_safe_as_userdata(value, NULL);

        if (userdata != NULL && userdata->is_typed) {
            return userdata->typed->name;
        }
    }

    return typeI_tostring(I, value.type);
}

bool valueI_is_type(morphine_instance_t I, const char *name, bool raw) {
    if (!raw && usertypeI_has(I, name)) {
        return true;
    }

#define type_if(s)                 if (strcmp(#s, name) == 0) { return true; };
#define mspec_type_object(i, n, s) type_if(s)
#define mspec_type_value(i, n, s)  type_if(s)

#include "morphine/core/type/specification.h"

#undef type_if
#undef mspec_type_object
#undef mspec_type_value

    return false;
}
