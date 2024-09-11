//
// Created by whyiskra on 27.12.23.
//

#undef sp_dispatch
#undef sp_case
#undef sp_continue

#define sp_dispatch(x) goto *sp_dispatch_tab[x];
#define sp_case(l) OL_##l:
#define sp_continue() morphinem_blk_start sp_fetch(); sp_dispatch(instruction.opcode) morphinem_blk_end

static const void *const sp_dispatch_tab[] = {
#define mis_instruction_label(n)             &&OL_MORPHINE_OPCODE_##n,
#define mis_instruction_args0(n, s)             mis_instruction_label(n)
#define mis_instruction_args1(n, s, a1)         mis_instruction_label(n)
#define mis_instruction_args2(n, s, a1, a2)     mis_instruction_label(n)
#define mis_instruction_args3(n, s, a1, a2, a3) mis_instruction_label(n)

#include "morphine/instruction/specification.h"

#undef mis_instruction_opcode
#undef mis_instruction_args0
#undef mis_instruction_args1
#undef mis_instruction_args2
#undef mis_instruction_args3
};
