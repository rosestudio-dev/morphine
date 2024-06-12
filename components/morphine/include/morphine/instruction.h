//
// Created by whyiskra on 08.11.23.
//

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "limits.h"

#define MORPHINE_OPCODES_START (MORPHINE_OPCODE_YIELD)
#define MORPHINE_OPCODES_COUNT (MORPHINE_OPCODE_LENGTH + 1)

typedef enum {
    MORPHINE_OPCODE_YIELD,           //                                                               yield
    MORPHINE_OPCODE_LOAD,            // [src (index), dest (slot)]                                    get from constant by (src) and set to (dest)
    MORPHINE_OPCODE_MOVE,            // [src (slot), dest (slot)]                                     get from (src) and set to (dest)
    MORPHINE_OPCODE_PARAM,           // [src (slot), dest (index)]                                    get from (src) and set to params by (dest)
    MORPHINE_OPCODE_ARG,             // [arg (index), dest (slot)]                                    get from args by (arg) and set to (dest)
    MORPHINE_OPCODE_CLEAR,           // [from (index), count (count)]                                 sets nil to slots starting (from) in amount (count)

    MORPHINE_OPCODE_ENV,             // [dest (slot)]                                                 move env to (dest)
    MORPHINE_OPCODE_SELF,            // [dest (slot)]                                                 move self to (dest)
    MORPHINE_OPCODE_RECURSION,       // [dest (slot)]                                                 move callable to (dest)

    MORPHINE_OPCODE_VECTOR,          // [dest (slot), size (count)]                                   create vector in (dest) with (size)
    MORPHINE_OPCODE_TABLE,           // [dest (slot)]                                                 create table in (dest)
    MORPHINE_OPCODE_GET,             // [container (slot), key (slot), dest (slot)]                   get from (container) by (key) to (dest)
    MORPHINE_OPCODE_SET,             // [container (slot), key (slot), src (slot)]                    set (src) to (container) by (key)

    MORPHINE_OPCODE_ITERATOR,        // [container (slot), dest (slot)]                               create iterator from (container) to (dest)
    MORPHINE_OPCODE_ITERATOR_INIT,   // [iterator (slot)]                                             init (iterator)
    MORPHINE_OPCODE_ITERATOR_HAS,    // [iterator (slot), dest (slot)]                                check next value of (iterator) to (dest)
    MORPHINE_OPCODE_ITERATOR_NEXT,   // [iterator (slot), dest (slot)]                                get next value of (iterator) to (dest)

    MORPHINE_OPCODE_JUMP,            // [position (pos)]                                              jump to (position)
    MORPHINE_OPCODE_JUMP_IF,         // [condition (slot), if_position (pos), else_position (pos)]    if (condition) is true jump to (if_position) else jump to (else_position)

    MORPHINE_OPCODE_GET_STATIC,      // [callable (slot), static (index), dest (slot)]                get static by (index) from (callable) to (dest)
    MORPHINE_OPCODE_SET_STATIC,      // [callable (slot), static (index), src (slot)]                 set (src) to static of (callable) by (index)

    MORPHINE_OPCODE_GET_CLOSURE,     // [closure (slot), closure (index), dest (slot)]                get closure by (index) from (closure) to (dest)
    MORPHINE_OPCODE_SET_CLOSURE,     // [closure (slot), closure (index), src (slot)]                 set (src) to (closure) by (index)

    MORPHINE_OPCODE_CLOSURE,         // [function (slot), params (count), dest (slot)]                create closure for (function) with count (params) in (dest)
    MORPHINE_OPCODE_CALL,            // [function (slot), params (count)]                             call (function) with count (params) and self as nil
    MORPHINE_OPCODE_SCALL,           // [function (slot), params (count), self (slot)]                call (function) with count (params) and (self)
    MORPHINE_OPCODE_LEAVE,           // [return (slot)]                                               leave with (return)
    MORPHINE_OPCODE_RESULT,          // [dest (slot)]                                                 set result value to (dest)

    MORPHINE_OPCODE_ADD,             // [lvalue (slot), rvalue (slot), dest (slot)]                   (lvalue) operator (rvalue) to (dest)
    MORPHINE_OPCODE_SUB,
    MORPHINE_OPCODE_MUL,
    MORPHINE_OPCODE_DIV,
    MORPHINE_OPCODE_MOD,
    MORPHINE_OPCODE_EQUAL,
    MORPHINE_OPCODE_LESS,
    MORPHINE_OPCODE_LESS_EQUAL,
    MORPHINE_OPCODE_AND,
    MORPHINE_OPCODE_OR,
    MORPHINE_OPCODE_CONCAT,

    MORPHINE_OPCODE_TYPE,            // [value (slot), dest (slot)]                                   get string type of (value) to (dest)
    MORPHINE_OPCODE_NEGATIVE,        // [value (slot), dest (slot)]                                   operator (lvalue) to (dest)
    MORPHINE_OPCODE_NOT,
    MORPHINE_OPCODE_REF,
    MORPHINE_OPCODE_DEREF,
    MORPHINE_OPCODE_LENGTH,
} morphine_opcode_t;

typedef struct {
    morphine_opcode_t opcode;
    ml_argument argument1;
    ml_argument argument2;
    ml_argument argument3;
    ml_line line;
} morphine_instruction_t;
