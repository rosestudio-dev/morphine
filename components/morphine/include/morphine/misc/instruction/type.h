//
// Created by whyiskra on 08.11.23.
//

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "morphine/limits.h"

#define MORPHINE_INSTRUCTION_ARGS_COUNT 3

#define MORPHINE_OPCODES_START (MTYPE_OPCODE_NOP)
#define MORPHINE_OPCODES_COUNT (MTYPE_OPCODE_LENGTH + 1)

typedef enum {
#define mspec_instruction_opcode(n)               MTYPE_OPCODE_##n,
#define mspec_instruction_args0(n, s)             mspec_instruction_opcode(n)
#define mspec_instruction_args1(n, s, a1)         mspec_instruction_opcode(n)
#define mspec_instruction_args2(n, s, a1, a2)     mspec_instruction_opcode(n)
#define mspec_instruction_args3(n, s, a1, a2, a3) mspec_instruction_opcode(n)

#include "specification.h"

#undef mspec_instruction_opcode
#undef mspec_instruction_args0
#undef mspec_instruction_args1
#undef mspec_instruction_args2
#undef mspec_instruction_args3
} mtype_opcode_t;

typedef struct {
    mtype_opcode_t opcode;

    union {
        ml_argument arguments[MORPHINE_INSTRUCTION_ARGS_COUNT];
        struct {
            ml_argument argument1;
            ml_argument argument2;
            ml_argument argument3;
        };
    };

    ml_line line;
} morphine_instruction_t;
