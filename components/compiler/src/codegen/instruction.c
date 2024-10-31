//
// Created by why-iskra on 18.08.2024.
//

#include "instruction.h"

#define dec_arg(t, n) , t n

#define sslot          struct instruction_slot
#define dslot          struct instruction_slot
#define size           size_t
#define position       anchor_t
#define constant_index size_t
#define param_index    size_t
#define argument_index size_t
#define static_index   size_t
#define closure_index  size_t
#define params_count   size_t

#define instruction_function(n, a1, a2, a3, names...) \
void codegen_instruction_##n(struct codegen_controller *C names) { \
    struct instruction_argument arg1 = { a1 }; \
    struct instruction_argument arg2 = { a2 }; \
    struct instruction_argument arg3 = { a3 }; \
    codegen_add_instruction(C, MORPHINE_OPCODE_##n, arg1, arg2, arg3); \
}

#define arg(t, d, v) .type = t, .d = v

#define mspec_instruction_args0(n, s)                instruction_function(n, arg(IAT_stub, value_stub, 0),          arg(IAT_stub, value_stub, 0),          arg(IAT_stub, value_stub, 0))
#define mspec_instruction_args1(n, s, ar1)           instruction_function(n, arg(IAT_##ar1, value_##ar1, ar1##_a1), arg(IAT_stub, value_stub, 0),          arg(IAT_stub, value_stub, 0),          dec_arg(ar1, ar1##_a1))
#define mspec_instruction_args2(n, s, ar1, ar2)      instruction_function(n, arg(IAT_##ar1, value_##ar1, ar1##_a1), arg(IAT_##ar2, value_##ar2, ar2##_a2), arg(IAT_stub, value_stub, 0),          dec_arg(ar1, ar1##_a1) dec_arg(ar2, ar2##_a2))
#define mspec_instruction_args3(n, s, ar1, ar2, ar3) instruction_function(n, arg(IAT_##ar1, value_##ar1, ar1##_a1), arg(IAT_##ar2, value_##ar2, ar2##_a2), arg(IAT_##ar3, value_##ar3, ar3##_a3), dec_arg(ar1, ar1##_a1) dec_arg(ar2, ar2##_a2) dec_arg(ar3, ar3##_a3))

#include "morphine/misc/instruction/specification.h"
