//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/gc/object.h"
#include "morphine/platform.h"
#include "morphine/utils/likely.h"

// create

#define valueI_create(t, d, x) ((struct value) { .type = VALUE_TYPE_##t, .d = (x) })

#define valueI_nil        valueI_create(NIL, nil, 0)
#define valueI_integer(x) valueI_create(INTEGER, integer, x)
#define valueI_decimal(x) valueI_create(DECIMAL, decimal, x)
#define valueI_boolean(x) valueI_create(BOOLEAN, boolean, x)
#define valueI_raw(x)     valueI_create(RAW, raw, (uintptr_t) (x))
#define valueI_object(x)  ({struct object *_obj = objectI_cast(x); ((struct value) { .type = (enum value_type) (_obj->type), .object.header = _obj });})

// is

#define valueI_is(t, x) ((x).type == VALUE_TYPE_##t)

#define valueI_is_nil(x)       valueI_is(NIL, x)
#define valueI_is_integer(x)   valueI_is(INTEGER, x)
#define valueI_is_decimal(x)   valueI_is(DECIMAL, x)
#define valueI_is_boolean(x)   valueI_is(BOOLEAN, x)
#define valueI_is_raw(x)       valueI_is(RAW, x)
#define valueI_is_userdata(x)  valueI_is(USERDATA, x)
#define valueI_is_string(x)    valueI_is(STRING, x)
#define valueI_is_table(x)     valueI_is(TABLE, x)
#define valueI_is_vector(x)    valueI_is(VECTOR, x)
#define valueI_is_closure(x)   valueI_is(CLOSURE, x)
#define valueI_is_coroutine(x) valueI_is(COROUTINE, x)
#define valueI_is_function(x)  valueI_is(FUNCTION, x)
#define valueI_is_native(x)    valueI_is(NATIVE, x)
#define valueI_is_exception(x) valueI_is(EXCEPTION, x)
#define valueI_is_stream(x)    valueI_is(STREAM, x)

#define valueI_is_object(x)   (typeI_isobj((x).type))
#define valueI_is_metatype(x) ({struct value _a = (x); (valueI_is_table(_a) || valueI_is_userdata(_a));})
#define valueI_is_iterable(x) ({struct value _a = (x); (valueI_is_table(_a) || valueI_is_vector(_a));})
#define valueI_is_callable(x) ({struct value _a = (x); (valueI_is_closure(_a) || valueI_is_function(_a) || valueI_is_native(_a));})

// as

#define valueI_as(t, x) ((x).t)

#define valueI_as_nil(x)       valueI_as(nil, x)
#define valueI_as_integer(x)   valueI_as(integer, x)
#define valueI_as_decimal(x)   valueI_as(decimal, x)
#define valueI_as_boolean(x)   valueI_as(boolean, x)
#define valueI_as_raw(x)       valueI_as(raw, x)
#define valueI_as_userdata(x)  valueI_as(object.userdata, x)
#define valueI_as_string(x)    valueI_as(object.string, x)
#define valueI_as_table(x)     valueI_as(object.table, x)
#define valueI_as_vector(x)    valueI_as(object.vector, x)
#define valueI_as_closure(x)   valueI_as(object.closure, x)
#define valueI_as_coroutine(x) valueI_as(object.coroutine, x)
#define valueI_as_function(x)  valueI_as(object.function, x)
#define valueI_as_native(x)    valueI_as(object.native, x)
#define valueI_as_exception(x) valueI_as(object.exception, x)
#define valueI_as_stream(x)    valueI_as(object.stream, x)
#define valueI_as_object(x)    valueI_as(object.header, x)

// safe as

#define valueI_as_or_default(t, x, o) ({struct value _a = (x); (mm_likely(valueI_is_##t(_a)) ? valueI_as_##t(_a) : (o));})

#define valueI_as_nil_or_default(x, o)       valueI_as_or_default(nil, x, o)
#define valueI_as_integer_or_default(x, o)   valueI_as_or_default(integer, x, o)
#define valueI_as_decimal_or_default(x, o)   valueI_as_or_default(decimal, x, o)
#define valueI_as_boolean_or_default(x, o)   valueI_as_or_default(boolean, x, o)
#define valueI_as_raw_or_default(x, o)       valueI_as_or_default(raw, x, o)
#define valueI_as_userdata_or_default(x, o)  valueI_as_or_default(userdata, x, o)
#define valueI_as_string_or_default(x, o)    valueI_as_or_default(string, x, o)
#define valueI_as_table_or_default(x, o)     valueI_as_or_default(table, x, o)
#define valueI_as_vector_or_default(x, o)    valueI_as_or_default(vector, x, o)
#define valueI_as_closure_or_default(x, o)   valueI_as_or_default(closure, x, o)
#define valueI_as_coroutine_or_default(x, o) valueI_as_or_default(coroutine, x, o)
#define valueI_as_function_or_default(x, o)  valueI_as_or_default(function, x, o)
#define valueI_as_native_or_default(x, o)    valueI_as_or_default(native, x, o)
#define valueI_as_exception_or_default(x, o) valueI_as_or_default(exception, x, o)
#define valueI_as_stream_or_default(x, o)    valueI_as_or_default(stream, x, o)
#define valueI_as_object_or_default(x, o)    valueI_as_or_default(object, x, o)

// as or code

#define valueI_as_or_cme(I, t, x, m) ({struct value _a = (x); if (mm_unlikely(!valueI_is_##t(_a))) throwI_error((I), (m)); valueI_as_##t(_a);})

#define valueI_as_nil_or_cme(I, x, m)       valueI_as_or_cme(I, nil, x, m)
#define valueI_as_integer_or_cme(I, x, m)   valueI_as_or_cme(I, integer, x, m)
#define valueI_as_decimal_or_cme(I, x, m)   valueI_as_or_cme(I, decimal, x, m)
#define valueI_as_boolean_or_cme(I, x, m)   valueI_as_or_cme(I, boolean, x, m)
#define valueI_as_raw_or_cme(I, x, m)       valueI_as_or_cme(I, raw, x, m)
#define valueI_as_userdata_or_cme(I, x, m)  valueI_as_or_cme(I, userdata, x, m)
#define valueI_as_string_or_cme(I, x, m)    valueI_as_or_cme(I, string, x, m)
#define valueI_as_table_or_cme(I, x, m)     valueI_as_or_cme(I, table, x, m)
#define valueI_as_vector_or_cme(I, x, m)    valueI_as_or_cme(I, vector, x, m)
#define valueI_as_closure_or_cme(I, x, m)   valueI_as_or_cme(I, closure, x, m)
#define valueI_as_coroutine_or_cme(I, x, m) valueI_as_or_cme(I, coroutine, x, m)
#define valueI_as_function_or_cme(I, x, m)  valueI_as_or_cme(I, function, x, m)
#define valueI_as_native_or_cme(I, x, m)    valueI_as_or_cme(I, native, x, m)
#define valueI_as_exception_or_cme(I, x, m) valueI_as_or_cme(I, exception, x, m)
#define valueI_as_stream_or_cme(I, x, m)    valueI_as_or_cme(I, stream, x, m)
#define valueI_as_object_or_cme(I, x, m)    valueI_as_or_cme(I, object, x, m)

// as or error

#define valueI_as_or_error(I, t, x) valueI_as_or_cme(I, t, x, "expected " #t)

#define valueI_as_nil_or_error(I, x)       valueI_as_or_error(I, nil, x)
#define valueI_as_integer_or_error(I, x)   valueI_as_or_error(I, integer, x)
#define valueI_as_decimal_or_error(I, x)   valueI_as_or_error(I, decimal, x)
#define valueI_as_boolean_or_error(I, x)   valueI_as_or_error(I, boolean, x)
#define valueI_as_raw_or_error(I, x)       valueI_as_or_error(I, raw, x)
#define valueI_as_userdata_or_error(I, x)  valueI_as_or_error(I, userdata, x)
#define valueI_as_string_or_error(I, x)    valueI_as_or_error(I, string, x)
#define valueI_as_table_or_error(I, x)     valueI_as_or_error(I, table, x)
#define valueI_as_vector_or_error(I, x)    valueI_as_or_error(I, vector, x)
#define valueI_as_closure_or_error(I, x)   valueI_as_or_error(I, closure, x)
#define valueI_as_coroutine_or_error(I, x) valueI_as_or_error(I, coroutine, x)
#define valueI_as_function_or_error(I, x)  valueI_as_or_error(I, function, x)
#define valueI_as_native_or_error(I, x)    valueI_as_or_error(I, native, x)
#define valueI_as_exception_or_error(I, x) valueI_as_or_error(I, exception, x)
#define valueI_as_stream_or_error(I, x)    valueI_as_or_error(I, stream, x)
#define valueI_as_object_or_error(I, x)    valueI_as_or_error(I, object, x)

// size

#define valueI_size(x) valueI_integer((ml_integer) (x))
#define valueI_as_namesize_or_error(I, x, m) mm_overflow_opc_ucast(ml_size, valueI_as_integer_or_error((I), (x)), throwI_error((I), "cannot convert integer to "m))
#define valueI_as_size_or_error(I, x)  valueI_as_namesize_or_error(I, x, "size")
#define valueI_as_index_or_error(I, x) valueI_as_namesize_or_error(I, x, "index")

// other

#define valueI_tobool(x) ({ struct value _v_bool = (x); !valueI_is_nil(_v_bool) && !valueI_is_exception(_v_bool) && valueI_as_boolean_or_default(_v_bool, true); })

// value

struct value {
    enum value_type type;
    union {
        void *nil;
        ml_integer integer;
        ml_decimal decimal;
        bool boolean;
        uintptr_t raw;

        union {
            struct object *header;
            struct closure *closure;
            struct native *native;
            struct function *function;
            struct exception *exception;
            struct coroutine *coroutine;
            struct string *string;
            struct table *table;
            struct vector *vector;
            struct userdata *userdata;
            struct stream *stream;
        } object;
    };
};

const char *valueI_type(morphine_instance_t, struct value, bool raw);
bool valueI_is_type(morphine_instance_t, const char *name, bool raw);

int valueI_compare(morphine_instance_t, struct value, struct value, bool different_types);
ml_hash valueI_hash(morphine_instance_t, struct value);
