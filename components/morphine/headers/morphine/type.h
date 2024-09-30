//
// Created by whyiskra on 3/15/24.
//

#pragma once

#define VALUE_TYPES_START (VALUE_TYPE_USERDATA)
#define VALUE_TYPES_COUNT (VALUE_TYPE_RAW + 1)

#define OBJ_TYPES_START (OBJ_TYPE_USERDATA)
#define OBJ_TYPES_COUNT (OBJ_TYPE_SIO + 1)

#define typeI_value_is_obj(x) ({ enum obj_type _vt = (enum obj_type) (x); (OBJ_TYPES_START <= _vt && _vt < OBJ_TYPES_COUNT); })

#define bridge(name) VALUE_TYPE_##name = OBJ_TYPE_##name
#define append(name, i) VALUE_TYPE_##name = (OBJ_TYPES_COUNT + (i))

enum obj_type {
    OBJ_TYPE_USERDATA = 0,
    OBJ_TYPE_STRING = 1,
    OBJ_TYPE_TABLE = 2,
    OBJ_TYPE_VECTOR = 3,
    OBJ_TYPE_CLOSURE = 4,
    OBJ_TYPE_COROUTINE = 5,
    OBJ_TYPE_FUNCTION = 6,
    OBJ_TYPE_NATIVE = 7,
    OBJ_TYPE_REFERENCE = 8,
    OBJ_TYPE_EXCEPTION = 9,
    OBJ_TYPE_ITERATOR = 10,
    OBJ_TYPE_SIO = 11,
};

enum value_type {
    bridge(USERDATA),
    bridge(STRING),
    bridge(TABLE),
    bridge(VECTOR),
    bridge(CLOSURE),
    bridge(COROUTINE),
    bridge(FUNCTION),
    bridge(NATIVE),
    bridge(REFERENCE),
    bridge(EXCEPTION),
    bridge(ITERATOR),
    bridge(SIO),

    append(NIL, 0),
    append(INTEGER, 1),
    append(DECIMAL, 2),
    append(BOOLEAN, 3),
    append(RAW, 4),
};

#undef bridge
#undef append
