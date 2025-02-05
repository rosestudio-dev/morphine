//
// Created by why-iskra on 02.06.2024.
//

#pragma once

#include "morphine/misc/instruction/type.h"
#include "morphine/object/function.h"

ml_size instructionI_opcode_args(mtype_opcode_t, bool *valid);
bool instructionI_validate(morphine_instruction_t, struct function *);
