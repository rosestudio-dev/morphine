//
// Created by why-iskra on 18.08.2024.
//

#pragma once

#include "controller.h"

#define slot   struct instruction_slot
#define count          size_t
#define anchor         anchor_t
#define constant_index size_t
#define param_index    size_t
#define argument_index size_t
#define static_index   size_t
#define closure_index  size_t
#define params_count   size_t

#define instruction_argument(n) , n
#define instruction_function(n, args...) void codegen_instruction_##n(struct codegen_controller * args);
#define instruction_args0(n)             instruction_function(n)
#define instruction_args1(n, a1)         instruction_function(n, instruction_argument(a1))
#define instruction_args2(n, a1, a2)     instruction_function(n, instruction_argument(a1) instruction_argument(a2))
#define instruction_args3(n, a1, a2, a3) instruction_function(n, instruction_argument(a1) instruction_argument(a2) instruction_argument(a3))

#include "instruction/specification.h"

#undef slot
#undef count
#undef anchor
#undef constant_index
#undef param_index
#undef argument_index
#undef static_index
#undef closure_index
#undef params_count

#undef instruction_argument
#undef instruction_function
#undef instruction_args0
#undef instruction_args1
#undef instruction_args2
#undef instruction_args3
