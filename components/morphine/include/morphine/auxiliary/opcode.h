//
// Created by why-iskra on 02.11.2024.
//

#pragma once

#include "morphine/platform.h"
#include "morphine/misc/instruction/type.h"

MORPHINE_AUX const char *maux_opcode_name(morphine_coroutine_t, morphine_opcode_t);
MORPHINE_AUX morphine_opcode_t maux_opcode_from_name(morphine_coroutine_t, const char *);
