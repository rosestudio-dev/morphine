//
// Created by why-iskra on 02.11.2024.
//

#include <string.h>
#include "morphine/auxiliary/opcode.h"
#include "morphine/api.h"

MORPHINE_AUX const char *maux_opcode_name(morphine_coroutine_t U, morphine_opcode_t opcode) {
    switch (opcode) {
#define mspec_instruction_opcode(n)               case MORPHINE_OPCODE_##n: return #n;
#define mspec_instruction_args0(n, s)             mspec_instruction_opcode(n)
#define mspec_instruction_args1(n, s, a1)         mspec_instruction_opcode(n)
#define mspec_instruction_args2(n, s, a1, a2)     mspec_instruction_opcode(n)
#define mspec_instruction_args3(n, s, a1, a2, a3) mspec_instruction_opcode(n)

#include "morphine/misc/instruction/specification.h"

#undef mspec_instruction_opcode
#undef mspec_instruction_args0
#undef mspec_instruction_args1
#undef mspec_instruction_args2
#undef mspec_instruction_args3
    }

    mapi_error(U, "undefined opcode");
}

MORPHINE_AUX morphine_opcode_t maux_opcode_from_name(morphine_coroutine_t U, const char *name) {
    for (morphine_opcode_t opcode = MORPHINE_OPCODES_START; opcode < MORPHINE_OPCODES_COUNT; opcode++) {
        if (strcmp(maux_opcode_name(U, opcode), name) == 0) {
            return opcode;
        }
    }

    mapi_error(U, "undefined opcode");
}
