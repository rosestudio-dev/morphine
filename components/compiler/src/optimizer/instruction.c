//
// Created by why-iskra on 29.09.2024.
//

#include "instruction.h"

enum a_type {
    AT_sslot,
    AT_dslot,
    AT_size,
    AT_position,
    AT_constant_index,
    AT_param_index,
    AT_argument_index,
    AT_static_index,
    AT_closure_index,
    AT_params_count,
    AT_stub,
};

static const enum a_type map[MORPHINE_OPCODES_COUNT][MORPHINE_INSTRUCTION_ARGS_COUNT] = {
#define mspec_instruction_args0(n, s)             { AT_stub, AT_stub, AT_stub },
#define mspec_instruction_args1(n, s, a1)         { AT_##a1, AT_stub, AT_stub },
#define mspec_instruction_args2(n, s, a1, a2)     { AT_##a1, AT_##a2, AT_stub },
#define mspec_instruction_args3(n, s, a1, a2, a3) { AT_##a1, AT_##a2, AT_##a3 },

#include "morphine/misc/instruction/specification.h"

#undef mspec_instruction_args0
#undef mspec_instruction_args1
#undef mspec_instruction_args2
#undef mspec_instruction_args3
};

struct instruction instruction_convert(morphine_coroutine_t U, morphine_instruction_t instruction) {
    size_t size = mapi_opcode(U, instruction.opcode);

    struct instruction result = {
        .origin = instruction,
        .has_dest = false,
        .src_count = 0,
        .dest = 0,
    };

    for (size_t i = 0; i < size; i++) {
        enum a_type type = map[instruction.opcode][i];
        if (type == AT_sslot) {
            result.srcs[result.src_count] = instruction.arguments[i];
            result.src_count++;
        } else if (type == AT_dslot) {
            result.dest = instruction.arguments[i];
            result.has_dest = true;
        }
    }

    if (result.has_dest && (result.src_count > 0)) {
        result.type = IT_PROCESSOR;
    } else if (!result.has_dest && (result.src_count > 0)) {
        result.type = IT_CONSUMER;
    } else if (result.has_dest) {
        result.type = IT_PRODUCER;
    } else {
        result.type = IT_CONTROL;
    }

    return result;
}

//morphine_instruction_t instruction_build(morphine_coroutine_t, struct instruction) {
//
//}
