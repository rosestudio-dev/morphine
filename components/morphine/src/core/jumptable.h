//
// Created by whyiskra on 27.12.23.
//

#undef sp_dispatch
#undef sp_case
#undef sp_continue

#define sp_dispatch(x) goto *sp_dispatch_tab[x];
#define sp_case(l)     OL_##l:
#define sp_continue()  semicolon_blk(sp_fetch(); sp_dispatch(instruction.opcode))

static const void *const sp_dispatch_tab[] = {
#define mspec_instruction_label(n)                &&OL_MTYPE_OPCODE_##n,
#define mspec_instruction_args0(n, s)             mspec_instruction_label(n)
#define mspec_instruction_args1(n, s, a1)         mspec_instruction_label(n)
#define mspec_instruction_args2(n, s, a1, a2)     mspec_instruction_label(n)
#define mspec_instruction_args3(n, s, a1, a2, a3) mspec_instruction_label(n)

#include "morphine/misc/instruction/specification.h"

#undef mspec_instruction_label
#undef mspec_instruction_args0
#undef mspec_instruction_args1
#undef mspec_instruction_args2
#undef mspec_instruction_args3
};
