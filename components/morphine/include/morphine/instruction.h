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
#define mis_instruction_opcode(n)            MORPHINE_OPCODE_##n,
#define mis_instruction_args0(n, s)             mis_instruction_opcode(n)
#define mis_instruction_args1(n, s, a1)         mis_instruction_opcode(n)
#define mis_instruction_args2(n, s, a1, a2)     mis_instruction_opcode(n)
#define mis_instruction_args3(n, s, a1, a2, a3) mis_instruction_opcode(n)

#include "instruction/specification.h"

#undef mis_instruction_opcode
#undef mis_instruction_args0
#undef mis_instruction_args1
#undef mis_instruction_args2
#undef mis_instruction_args3
} morphine_opcode_t;

typedef struct {
    morphine_opcode_t opcode;
    ml_argument argument1;
    ml_argument argument2;
    ml_argument argument3;
    ml_line line;
} morphine_instruction_t;
