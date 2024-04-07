//
// Created by whyiskra on 16.12.23.
//

#include "morphine/core/instruction.h"

bool instructionI_validate(
    instruction_t instruction,
    size_t arguments_count,
    size_t slots_count,
    size_t params_count,
    size_t closures_count,
    size_t statics_count,
    size_t constants_count
) {
#define arg_type(a, t) if(instruction.a.type != (t)) goto error;
#define arg_type_count(a, t, s) if(instruction.a.type != (t) || instruction.a.value >= (s)) goto error;
#define arg_type_size(a, t, s) if(instruction.a.type != (t) || instruction.a.value > (s)) goto error;

#define arg_position(a) arg_type(a, ARGUMENT_TYPE_POSITION)
#define arg_closure(a) arg_type_count(a, ARGUMENT_TYPE_CLOSURE, closures_count)
#define arg_static(a) arg_type_count(a, ARGUMENT_TYPE_STATIC, statics_count)
#define arg_slot(a) arg_type_count(a, ARGUMENT_TYPE_SLOT, slots_count)
#define arg_constant(a) arg_type_count(a, ARGUMENT_TYPE_CONSTANT, constants_count)
#define arg_param(a) arg_type_count(a, ARGUMENT_TYPE_PARAM, params_count)
#define arg_argument(a) arg_type_count(a, ARGUMENT_TYPE_ARG, arguments_count)
#define arg_params_count(a) arg_type_size(a, ARGUMENT_TYPE_COUNT, params_count)
#define arg_undefined(a) arg_type(a, ARGUMENT_TYPE_UNDEFINED)

    switch (instruction.opcode) {
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
        case OPCODE_SET_STATIC: {
            arg_static(argument1)
            arg_slot(argument2)
            arg_undefined(argument3)
            return true;
        }
        case OPCODE_GET_CLOSURE:
        case OPCODE_SET_CLOSURE: {
            arg_closure(argument1)
            arg_slot(argument2)
            arg_undefined(argument3)
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
