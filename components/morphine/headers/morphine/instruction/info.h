//
// Created by why-iskra on 02.06.2024.
//

#pragma once

#include "morphine/instruction.h"

uint8_t instructionI_opcode_args(morphine_opcode_t, bool *valid);

bool instructionI_validate(
    morphine_instruction_t,
    size_t arguments_count,
    size_t slots_count,
    size_t params_count,
    size_t constants_count
);