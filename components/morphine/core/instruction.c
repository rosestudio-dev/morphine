//
// Created by whyiskra on 16.12.23.
//

#include "morphine/core/instruction.h"

const uint8_t instructionI_opcode_args[OPCODES_COUNT] = {
    0, // OPCODE_YIELD
    2, // OPCODE_LOAD
    2, // OPCODE_MOVE
    2, // OPCODE_PARAM
    2, // OPCODE_ARG
    2, // OPCODE_CLEAR

    1, // OPCODE_ENV
    1, // OPCODE_SELF
    1, // OPCODE_RECURSION

    2, // OPCODE_VECTOR
    1, // OPCODE_TABLE
    3, // OPCODE_GET
    3, // OPCODE_SET

    2, // OPCODE_ITERATOR
    1, // OPCODE_ITERATOR_INIT
    2, // OPCODE_ITERATOR_HAS
    2, // OPCODE_ITERATOR_NEXT

    1, // OPCODE_JUMP
    3, // OPCODE_JUMP_IF

    3, // OPCODE_GET_STATIC
    3, // OPCODE_SET_STATIC

    3, // OPCODE_GET_CLOSURE
    3, // OPCODE_SET_CLOSURE

    3, // OPCODE_CLOSURE
    2, // OPCODE_CALL
    3, // OPCODE_SCALL
    1, // OPCODE_LEAVE
    1, // OPCODE_RESULT

    3, // OPCODE_ADD
    3, // OPCODE_SUB
    3, // OPCODE_MUL
    3, // OPCODE_DIV
    3, // OPCODE_MOD
    3, // OPCODE_EQUAL
    3, // OPCODE_LESS
    3, // OPCODE_LESS_EQUAL,
    3, // OPCODE_AND
    3, // OPCODE_OR
    3, // OPCODE_CONCAT

    2, // OPCODE_TYPE
    2, // OPCODE_NEGATIVE
    2, // OPCODE_NOT
    2, // OPCODE_REF
    2, // OPCODE_DEREF
    2, // OPCODE_LENGTH
};

bool instructionI_validate(
    instruction_t instruction,
    size_t arguments_count,
    size_t slots_count,
    size_t params_count,
    size_t constants_count
) {
#define arg_type_count(a, s) if(instruction.a.value >= (s)) goto error;
#define arg_type_size(a, s) if(instruction.a.value > (s)) goto error;

#define arg_position(a)
#define arg_slot(a) arg_type_count(a, slots_count)
#define arg_constant(a) arg_type_count(a, constants_count)
#define arg_param(a) arg_type_count(a, params_count)
#define arg_argument(a) arg_type_count(a, arguments_count)
#define arg_params_count(a) arg_type_size(a, params_count)
#define arg_count(a)
#define arg_index(a)
#define arg_undefined(a)

    switch (instruction.opcode) {
        case OPCODE_CLEAR: {
            size_t from = instruction.argument1.value;
            size_t count = instruction.argument2.value;

            if (count > ARGUMENT_MAX_VALUE - from) {
                goto error;
            }

            if (from >= slots_count || from + count > slots_count) {
                goto error;
            }

            arg_undefined(argument3)
            return true;
        }
        case OPCODE_YIELD: {
            arg_undefined(argument1)
            arg_undefined(argument2)
            arg_undefined(argument3)
            return true;
        }
        case OPCODE_LOAD: {
            arg_constant(argument1)
            arg_slot(argument2)
            arg_undefined(argument3)
            return true;
        }
        case OPCODE_PARAM: {
            arg_slot(argument1)
            arg_param(argument2)
            arg_undefined(argument3)
            return true;
        }
        case OPCODE_ARG: {
            arg_argument(argument1)
            arg_slot(argument2)
            arg_undefined(argument3)
            return true;
        }
        case OPCODE_JUMP: {
            arg_position(argument1)
            arg_undefined(argument2)
            arg_undefined(argument3)
            return true;
        }
        case OPCODE_JUMP_IF: {
            arg_slot(argument1)
            arg_position(argument2)
            arg_position(argument3)
            return true;
        }
        case OPCODE_GET_STATIC:
        case OPCODE_SET_STATIC:
        case OPCODE_GET_CLOSURE:
        case OPCODE_SET_CLOSURE: {
            arg_slot(argument1)
            arg_index(argument2)
            arg_slot(argument3)
            return true;
        }
        case OPCODE_CLOSURE:
        case OPCODE_SCALL: {
            arg_slot(argument1)
            arg_params_count(argument2)
            arg_slot(argument3)
            return true;
        }
        case OPCODE_CALL: {
            arg_slot(argument1)
            arg_params_count(argument2)
            arg_undefined(argument3)
            return true;
        }
        case OPCODE_VECTOR: {
            arg_slot(argument1)
            arg_count(argument2)
            arg_undefined(argument3)
            return true;
        }
        case OPCODE_ITERATOR_INIT:
        case OPCODE_RECURSION:
        case OPCODE_ENV:
        case OPCODE_SELF:
        case OPCODE_TABLE:
        case OPCODE_LEAVE:
        case OPCODE_RESULT: {
            arg_slot(argument1)
            arg_undefined(argument2)
            arg_undefined(argument3)
            return true;
        }
        case OPCODE_GET:
        case OPCODE_SET:
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_MOD:
        case OPCODE_EQUAL:
        case OPCODE_LESS:
        case OPCODE_LESS_EQUAL:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_CONCAT: {
            arg_slot(argument1)
            arg_slot(argument2)
            arg_slot(argument3)
            return true;
        }
        case OPCODE_ITERATOR:
        case OPCODE_ITERATOR_HAS:
        case OPCODE_ITERATOR_NEXT:
        case OPCODE_MOVE:
        case OPCODE_TYPE:
        case OPCODE_NEGATIVE:
        case OPCODE_NOT:
        case OPCODE_REF:
        case OPCODE_DEREF:
        case OPCODE_LENGTH: {
            arg_slot(argument1)
            arg_slot(argument2)
            arg_undefined(argument3)
            return true;
        }
    }

error:
    return false;
}
