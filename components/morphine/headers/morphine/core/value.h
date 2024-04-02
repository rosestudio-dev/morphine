//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "morphine/platform.h"
#include "morphine/utils/likely.h"
#include "morphine/object.h"

// region create

#define valueI_create(t, d, x) ((struct value) { .type = VALUE_TYPE_##t, .d = (x) })

#define valueI_nil        valueI_create(NIL, nil, 0)
#define valueI_integer(x) valueI_create(INTEGER, integer, x)
#define valueI_decimal(x) valueI_create(DECIMAL, decimal, x)
#define valueI_boolean(x) valueI_create(BOOLEAN, boolean, x)
#define valueI_raw(x)     valueI_create(RAW, raw, x)

#define valueI_object(x)  ({struct object *_obj = objectI_cast(x); ((struct value) { .type = (enum value_type) (_obj->type), .object.header = _obj });})

#define valueI_object_or_panic(I, S, x)  ({struct object *_obj = objectI_cast(x); if(unlikely(_obj == NULL)) throwI_message_panic((I), (S), "Object is null"); ((struct value) { .type = (enum value_type) (_obj->type), .object.header = _obj });})

// endregion

// region is

#define valueI_is(t, x) ((x).type == VALUE_TYPE_##t)

#define valueI_is_nil(x)       valueI_is(NIL, x)
#define valueI_is_integer(x)   valueI_is(INTEGER, x)
#define valueI_is_decimal(x)   valueI_is(DECIMAL, x)
#define valueI_is_boolean(x)   valueI_is(BOOLEAN, x)
#define valueI_is_raw(x)       valueI_is(RAW, x)
#define valueI_is_userdata(x)  valueI_is(USERDATA, x)
#define valueI_is_string(x)    valueI_is(STRING, x)
#define valueI_is_table(x)     valueI_is(TABLE, x)
#define valueI_is_closure(x)   valueI_is(CLOSURE, x)
#define valueI_is_state(x)     valueI_is(STATE, x)
#define valueI_is_proto(x)     valueI_is(PROTO, x)
#define valueI_is_native(x)    valueI_is(NATIVE, x)
#define valueI_is_reference(x) valueI_is(REFERENCE, x)
#define valueI_is_iterator(x)  valueI_is(ITERATOR, x)

#define valueI_is_object(x)  typeI_value_is_obj((x).type)

// endregion

// region as

#define valueI_as(t, x) ((x).t)

#define valueI_as_nil(x)       valueI_as(nil, x)
#define valueI_as_integer(x)   valueI_as(integer, x)
#define valueI_as_decimal(x)   valueI_as(decimal, x)
#define valueI_as_boolean(x)   valueI_as(boolean, x)
#define valueI_as_raw(x)       valueI_as(raw, x)
#define valueI_as_userdata(x)  valueI_as(object.userdata, x)
#define valueI_as_string(x)    valueI_as(object.string, x)
#define valueI_as_table(x)     valueI_as(object.table, x)
#define valueI_as_closure(x)   valueI_as(object.closure, x)
#define valueI_as_state(x)     valueI_as(object.state, x)
#define valueI_as_proto(x)     valueI_as(object.proto, x)
#define valueI_as_native(x)    valueI_as(object.native, x)
#define valueI_as_reference(x) valueI_as(object.reference, x)
#define valueI_as_iterator(x)  valueI_as(object.iterator, x)

#define valueI_as_object(x) valueI_as(object.header, x)

// endregion

// region safe_as

#define valueI_safe_as(t, x, o) ({struct value _a = (x); (likely(valueI_is_##t(_a)) ? valueI_as_##t(_a) : (o));})

#define valueI_safe_as_nil(x, o)       valueI_safe_as(nil, x, o)
#define valueI_safe_as_integer(x, o)   valueI_safe_as(integer, x, o)
#define valueI_safe_as_decimal(x, o)   valueI_safe_as(decimal, x, o)
#define valueI_safe_as_boolean(x, o)   valueI_safe_as(boolean, x, o)
#define valueI_safe_as_raw(x, o)       valueI_safe_as(raw, x, o)
#define valueI_safe_as_userdata(x, o)  valueI_safe_as(userdata, x, o)
#define valueI_safe_as_string(x, o)    valueI_safe_as(string, x, o)
#define valueI_safe_as_table(x, o)     valueI_safe_as(table, x, o)
#define valueI_safe_as_closure(x, o)   valueI_safe_as(closure, x, o)
#define valueI_safe_as_state(x, o)     valueI_safe_as(state, x, o)
#define valueI_safe_as_proto(x, o)     valueI_safe_as(proto, x, o)
#define valueI_safe_as_native(x, o)    valueI_safe_as(native, x, o)
#define valueI_safe_as_reference(x, o) valueI_safe_as(reference, x, o)
#define valueI_safe_as_iterator(x, o)  valueI_safe_as(iterator, x, o)

#define valueI_safe_as_object(x, o) ({struct value _a = (x); (likely(valueI_is_object(_a)) ? valueI_as_object(_a) : (o));})

// endregion

// region as_or_error

#define valueI_as_or_error(S, t, x) ({struct value _a = (x); if(unlikely(!valueI_is_##t(_a))) throwI_message_error((S), "Expected " #t); valueI_as_##t(_a);})

#define valueI_as_nil_or_error(S, x)       valueI_as_or_error(S, nil, x)
#define valueI_as_integer_or_error(S, x)   valueI_as_or_error(S, integer, x)
#define valueI_as_decimal_or_error(S, x)   valueI_as_or_error(S, decimal, x)
#define valueI_as_boolean_or_error(S, x)   valueI_as_or_error(S, boolean, x)
#define valueI_as_raw_or_error(S, x)       valueI_as_or_error(S, raw, x)
#define valueI_as_userdata_or_error(S, x)  valueI_as_or_error(S, userdata, x)
#define valueI_as_string_or_error(S, x)    valueI_as_or_error(S, string, x)
#define valueI_as_table_or_error(S, x)     valueI_as_or_error(S, table, x)
#define valueI_as_closure_or_error(S, x)   valueI_as_or_error(S, closure, x)
#define valueI_as_state_or_error(S, x)     valueI_as_or_error(S, state, x)
#define valueI_as_proto_or_error(S, x)     valueI_as_or_error(S, proto, x)
#define valueI_as_native_or_error(S, x)    valueI_as_or_error(S, native, x)
#define valueI_as_reference_or_error(S, x) valueI_as_or_error(S, reference, x)
#define valueI_as_iterator_or_error(S, x)  valueI_as_or_error(S, iterator, x)

#define valueI_as_object_or_error(S, x) ({struct value _a = (x); if(unlikely(!valueI_is_object(_a))) throwI_message_error((S), "Expected object"); valueI_as_object(_a);})

// endregion

#define valueI_size2integer(S, x) ({size_t _s = (x); if(unlikely(_s > MORPHINE_INTEGER_MAX)) throwI_errorf((S), "Cannot convert %zu to integer", _s); valueI_integer((morphine_integer_t) _s);})
#define valueI_integer2size(S, x) ({morphine_integer_t _i = (x); if(unlikely(_i < 0 || ((size_t) _i) > SIZE_MAX)) throwI_errorf((S), "Cannot convert %"MORPHINE_INTEGER_PRINT" to size", _i); ((size_t) _i);})

#define valueI_pair(k, v) ((struct pair) { .key = (k), .value = (v) })

struct value {
    enum value_type type;
    union {
        void *nil;
        morphine_integer_t integer;
        morphine_decimal_t decimal;
        bool boolean;
        void *raw;

        union {
            struct object *header;
            struct closure *closure;
            struct native *native;
            struct proto *proto;
            struct reference *reference;
            struct state *state;
            struct string *string;
            struct table *table;
            struct userdata *userdata;
            struct iterator *iterator;
        } object;
    };
};

struct pair {
    struct value key;
    struct value value;
};

const char *valueI_type2string(morphine_instance_t, enum value_type type);
enum value_type valueI_string2type(morphine_state_t, const char *name);

bool valueI_equal(morphine_instance_t, struct value a, struct value b);
struct value valueI_value2string(morphine_instance_t, struct value value);
struct value valueI_value2integer(morphine_state_t, struct value value);
struct value valueI_value2decimal(morphine_state_t, struct value value);
struct value valueI_value2boolean(morphine_state_t, struct value value);
