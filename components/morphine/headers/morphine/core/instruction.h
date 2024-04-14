//
// Created by whyiskra on 08.11.23.
//

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define OPCODES_START (OPCODE_YIELD)
#define OPCODES_COUNT (OPCODE_LENGTH + 1)

typedef enum {
    OPCODE_YIELD,           //                                                               yield
    OPCODE_LOAD,            // [src (const), dest (slot)]                                    get from (src) and set to (dest)
    OPCODE_MOVE,            // [src (slot), dest (slot)]                                     get from (src) and set to (dest)
    OPCODE_PARAM,           // [src (slot), dest (param)]                                    get from (src) and set to (param)
    OPCODE_ARG,             // [src (arg), dest (slot)]                                      get from (src) and set to (dest)

    OPCODE_ENV,             // [dest (slot)]                                                 move env to (dest)
    OPCODE_SELF,            // [dest (slot)]                                                 move self to (dest)
    OPCODE_RECURSION,       // [dest (slot)]                                                 move callable to (dest)

    OPCODE_TABLE,           // [dest (slot)]                                                 create table in (dest)
    OPCODE_GET,             // [container (slot), key (slot), dest (slot)]                   get from (container) by (key) to (dest)
    OPCODE_SET,             // [container (slot), key (slot), src (slot)]                    set (src) to (container) by (key)

    OPCODE_ITERATOR,        // [container (slot), dest (slot)]                               create iterator from (container) to (dest)
    OPCODE_ITERATOR_INIT,   // [iterator (slot)]                                             init (iterator)
    OPCODE_ITERATOR_HAS,    // [iterator (slot), dest (slot)]                                check next value of (iterator) to (dest)
    OPCODE_ITERATOR_NEXT,   // [iterator (slot), dest (slot)]                                get next value of (iterator) to (dest)

    OPCODE_JUMP,            // [position (pos)]                                              jump to (position)
    OPCODE_JUMP_IF,         // [condition (slot), if_position (pos), else_position (pos)]    if (condition) is true jump to (if_position) else jump to (else_position)

    OPCODE_GET_STATIC,      // [static (id), dest (slot)]                                    get from (static) to (dest)
    OPCODE_SET_STATIC,      // [static (id), src (slot)]                                     set (src) to (static)

    OPCODE_GET_CLOSURE,     // [closure (id), dest (slot)]                                   get from (closure) to (dest)
    OPCODE_SET_CLOSURE,     // [closure (id), src (slot)]                                    set (src) to (closure)

    OPCODE_CLOSURE,         // [function (slot), params (num), dest (slot)]                  create closure for (function) with params (size) in (dest)
    OPCODE_CALL,            // [function (slot), params (num)]                               call (function) with params (size) and self as nil
    OPCODE_SCALL,           // [function (slot), params (num), self (slot)]                  call (function) with params (size) and (self)
    OPCODE_LEAVE,           // [return (slot)]                                               leave with (return)
    OPCODE_RESULT,          // [dest (slot)]                                                 set result value to (dest)

    OPCODE_ADD,             // [lvalue (slot), rvalue (slot), dest (slot)]                   (lvalue) operator (rvalue) to (dest)
    OPCODE_SUB,
    OPCODE_MUL,
    OPCODE_DIV,
    OPCODE_MOD,
    OPCODE_EQUAL,
    OPCODE_LESS,
    OPCODE_LESS_EQUAL,
    OPCODE_AND,
    OPCODE_OR,
    OPCODE_CONCAT,

    OPCODE_TYPE,            // [value (slot), dest (slot)]                                   get string type of (value) to (dest)
    OPCODE_NEGATIVE,        // [value (slot), dest (slot)]                                   operator (lvalue) to (dest)
    OPCODE_NOT,
    OPCODE_REF,
    OPCODE_DEREF,
    OPCODE_LENGTH,
} opcode_t;

extern const uint8_t instructionI_opcode_args[OPCODES_COUNT];

typedef struct {
    uint16_t value;
} argument_t;

typedef struct instruction_t {
    uint32_t line;
    opcode_t opcode;
    argument_t argument1;
    argument_t argument2;
    argument_t argument3;
} instruction_t;

bool instructionI_validate(
    instruction_t instruction,
    size_t arguments_count,
    size_t slots_count,
    size_t params_count,
    size_t closures_count,
    size_t statics_count,
    size_t constants_count
);

static inline bool instructionI_is_valid_opcode(opcode_t opcode) {
    return OPCODES_START <= opcode && opcode < OPCODES_COUNT;
}
