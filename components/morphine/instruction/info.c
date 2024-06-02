//
// Created by whyiskra on 16.12.23.
//

#include "morphine/instruction/info.h"

static const uint8_t opcode_args[MORPHINE_OPCODES_COUNT] = {
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

uint8_t instructionI_opcode_args(morphine_opcode_t opcode, bool *valid) {
    if (MORPHINE_OPCODES_START <= opcode && opcode < MORPHINE_OPCODES_COUNT) {
        if (valid != NULL) {
            *valid = true;
        }

        return opcode_args[opcode];
    }

    if (valid != NULL) {
        *valid = false;
    }

    return 0;
}

bool instructionI_validate(
    morphine_instruction_t instruction,
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
        case MORPHINE_OPCODE_CLEAR: {
            size_t from = instruction.argument1.value;
            size_t count = instruction.argument2.value;

            if (count > MORPHINE_ARGUMENT_MAX_VALUE - from) {
                goto error;
            }

            if (from >= slots_count || from + count > slots_count) {
                goto error;
            }

            arg_undefined(argument3)
            return true;
        }
        case MORPHINE_OPCODE_YIELD: {
            arg_undefined(argument1)
            arg_undefined(argument2)
            arg_undefined(argument3)
            return true;
        }
        case MORPHINE_OPCODE_LOAD: {
            arg_constant(argument1)
            arg_slot(argument2)
            arg_undefined(argument3)
            return true;
        }
        case MORPHINE_OPCODE_PARAM: {
            arg_slot(argument1)
            arg_param(argument2)
            arg_undefined(argument3)
            return true;
        }
        case MORPHINE_OPCODE_ARG: {
            arg_argument(argument1)
            arg_slot(argument2)
            arg_undefined(argument3)
            return true;
        }
        case MORPHINE_OPCODE_JUMP: {
            arg_position(argument1)
            arg_undefined(argument2)
            arg_undefined(argument3)
            return true;
        }
        case MORPHINE_OPCODE_JUMP_IF: {
            arg_slot(argument1)
            arg_position(argument2)
            arg_position(argument3)
            return true;
        }
        case MORPHINE_OPCODE_GET_STATIC:
        case MORPHINE_OPCODE_SET_STATIC:
        case MORPHINE_OPCODE_GET_CLOSURE:
        case MORPHINE_OPCODE_SET_CLOSURE: {
            arg_slot(argument1)
            arg_index(argument2)
            arg_slot(argument3)
            return true;
        }
        case MORPHINE_OPCODE_CLOSURE:
        case MORPHINE_OPCODE_SCALL: {
            arg_slot(argument1)
            arg_params_count(argument2)
            arg_slot(argument3)
            return true;
        }
        case MORPHINE_OPCODE_CALL: {
            arg_slot(argument1)
            arg_params_count(argument2)
            arg_undefined(argument3)
            return true;
        }
        case MORPHINE_OPCODE_VECTOR: {
            arg_slot(argument1)
            arg_count(argument2)
            arg_undefined(argument3)
            return true;
        }
        case MORPHINE_OPCODE_ITERATOR_INIT:
        case MORPHINE_OPCODE_RECURSION:
        case MORPHINE_OPCODE_ENV:
        case MORPHINE_OPCODE_SELF:
        case MORPHINE_OPCODE_TABLE:
        case MORPHINE_OPCODE_LEAVE:
        case MORPHINE_OPCODE_RESULT: {
            arg_slot(argument1)
            arg_undefined(argument2)
            arg_undefined(argument3)
            return true;
        }
        case MORPHINE_OPCODE_GET:
        case MORPHINE_OPCODE_SET:
        case MORPHINE_OPCODE_ADD:
        case MORPHINE_OPCODE_SUB:
        case MORPHINE_OPCODE_MUL:
        case MORPHINE_OPCODE_DIV:
        case MORPHINE_OPCODE_MOD:
        case MORPHINE_OPCODE_EQUAL:
        case MORPHINE_OPCODE_LESS:
        case MORPHINE_OPCODE_LESS_EQUAL:
        case MORPHINE_OPCODE_AND:
        case MORPHINE_OPCODE_OR:
        case MORPHINE_OPCODE_CONCAT: {
            arg_slot(argument1)
            arg_slot(argument2)
            arg_slot(argument3)
            return true;
        }
        case MORPHINE_OPCODE_ITERATOR:
        case MORPHINE_OPCODE_ITERATOR_HAS:
        case MORPHINE_OPCODE_ITERATOR_NEXT:
        case MORPHINE_OPCODE_MOVE:
        case MORPHINE_OPCODE_TYPE:
        case MORPHINE_OPCODE_NEGATIVE:
        case MORPHINE_OPCODE_NOT:
        case MORPHINE_OPCODE_REF:
        case MORPHINE_OPCODE_DEREF:
        case MORPHINE_OPCODE_LENGTH: {
            arg_slot(argument1)
            arg_slot(argument2)
            arg_undefined(argument3)
            return true;
        }
    }

error:
    return false;
}
