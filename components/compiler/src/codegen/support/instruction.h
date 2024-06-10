//
// Created by why-iskra on 08.06.2024.
//

#pragma once

#include "controller.h"

#define instr_arg(n)               , struct codegen_argument_##n
#define instr_func(n, args...)     void codegen_instruction_##n(struct codegen_controller * args);
#define instr_func0(n)             instr_func(n)
#define instr_func1(n, a1)         instr_func(n, instr_arg(a1))
#define instr_func2(n, a1, a2)     instr_func(n, instr_arg(a1) instr_arg(a2))
#define instr_func3(n, a1, a2, a3) instr_func(n, instr_arg(a1) instr_arg(a2) instr_arg(a3))

#include "instruction/impl.h"

#undef instr_arg
#undef instr_func
#undef instr_func0
#undef instr_func1
#undef instr_func2
#undef instr_func3
