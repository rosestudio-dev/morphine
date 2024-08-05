//
// Created by why-iskra on 18.08.2024.
//

#pragma once

#include "controller.h"

#define slot   struct instruction_slot
#define index  size_t
#define count  size_t
#define anchor anchor_t

#define instr_arg(n) , n
#define instr_func(n, args...)     void codegen_instruction_##n(struct codegen_controller * args);
#define instr_func0(n)             instr_func(n)
#define instr_func1(n, a1)         instr_func(n, instr_arg(a1))
#define instr_func2(n, a1, a2)     instr_func(n, instr_arg(a1) instr_arg(a2))
#define instr_func3(n, a1, a2, a3) instr_func(n, instr_arg(a1) instr_arg(a2) instr_arg(a3))

#include "instruction/specification.h"

#undef slot
#undef index
#undef count
#undef anchor

#undef instr_arg
#undef instr_func
#undef instr_func0
#undef instr_func1
#undef instr_func2
#undef instr_func3
