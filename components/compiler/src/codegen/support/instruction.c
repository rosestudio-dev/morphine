//
// Created by why-iskra on 08.06.2024.
//

#include "instruction.h"

#include <morphine/instruction.h>

#define instr_func(n, a1, a2, a3, names...) \
void codegen_instruction_##n(struct codegen_controller *N names) { \
    struct codegen_argument arg1 = { a1 }; \
    struct codegen_argument arg2 = { a2 }; \
    struct codegen_argument arg3 = { a3 }; \
    codegen_instruction(N, MORPHINE_OPCODE_##n, arg1, arg2, arg3); \
}

#define arg(t, v) .type = CAT_##t, .value.t = v
#define dec_arg(t, n) , struct codegen_argument_##t n

#define instr_func0(n)                instr_func(n, arg(stub, 0), arg(stub, 0), arg(stub, 0))
#define instr_func1(n, ar1)           instr_func(n, arg(ar1, a1), arg(stub, 0), arg(stub, 0), dec_arg(ar1, a1))
#define instr_func2(n, ar1, ar2)      instr_func(n, arg(ar1, a1), arg(ar2, a2), arg(stub, 0), dec_arg(ar1, a1) dec_arg(ar2, a2))
#define instr_func3(n, ar1, ar2, ar3) instr_func(n, arg(ar1, a1), arg(ar2, a2), arg(ar3, a3), dec_arg(ar1, a1) dec_arg(ar2, a2) dec_arg(ar3, a3))

#include "instruction/impl.h"
