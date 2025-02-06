//
// Created by why-iskra on 14.12.2024.
//

#pragma once

#define mstr_opcode(s) static const char *const mstr_opcode_##s = #s;
#define mspec_instruction_args0(n, s)             mstr_opcode(s)
#define mspec_instruction_args1(n, s, a1)         mstr_opcode(s)
#define mspec_instruction_args2(n, s, a1, a2)     mstr_opcode(s)
#define mspec_instruction_args3(n, s, a1, a2, a3) mstr_opcode(s)

#include "specification.h"

#undef mstr_opcode
#undef mspec_instruction_args0
#undef mspec_instruction_args1
#undef mspec_instruction_args2
#undef mspec_instruction_args3
