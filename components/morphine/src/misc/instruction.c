//
// Created by whyiskra on 16.12.23.
//

#include "morphine/misc/instruction.h"

static const ml_size opcode_args[] = {
#define mspec_instruction_args0(n, s)             0,
#define mspec_instruction_args1(n, s, a1)         1,
#define mspec_instruction_args2(n, s, a1, a2)     2,
#define mspec_instruction_args3(n, s, a1, a2, a3) 3,

#include "morphine/misc/instruction/specification.h"

#undef mspec_instruction_args0
#undef mspec_instruction_args1
#undef mspec_instruction_args2
#undef mspec_instruction_args3
};

ml_size instructionI_opcode_args(morphine_opcode_t opcode, bool *valid) {
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
    struct function *function
) {
    if (function == NULL) {
        goto error;
    }

#define arg_type_index(a, s) do { if (instruction.argument##a >= (s)) goto error; } while (0)
#define arg_type_size(a, s)  do { if (instruction.argument##a > (s)) goto error; } while (0)

#define arg_undefined(a)
#define arg_position(a)
#define arg_size(a)

#define arg_closure_index(a)
#define arg_static_index(a)

#define arg_sslot(a)          arg_type_index(a, function->slots_count)
#define arg_dslot(a)          arg_type_index(a, function->slots_count)
#define arg_constant_index(a) arg_type_index(a, function->constants_count)
#define arg_param_index(a)    arg_type_index(a, function->params_count)
#define arg_argument_index(a) arg_type_index(a, function->arguments_count)
#define arg_params_count(a)   arg_type_size(a, function->params_count)


    switch (instruction.opcode) {
#define mspec_instruction_args0(n, s)             case MORPHINE_OPCODE_##n: arg_undefined(1); arg_undefined(2); arg_undefined(3); return true;
#define mspec_instruction_args1(n, s, a1)         case MORPHINE_OPCODE_##n: arg_##a1(1);      arg_undefined(2); arg_undefined(3); return true;
#define mspec_instruction_args2(n, s, a1, a2)     case MORPHINE_OPCODE_##n: arg_##a1(1);      arg_##a2(2);      arg_undefined(3); return true;
#define mspec_instruction_args3(n, s, a1, a2, a3) case MORPHINE_OPCODE_##n: arg_##a1(1);      arg_##a2(2);      arg_##a3(3);      return true;

#include "morphine/misc/instruction/specification.h"

#undef mspec_instruction_args0
#undef mspec_instruction_args1
#undef mspec_instruction_args2
#undef mspec_instruction_args3
    }

error:
    return false;
}
