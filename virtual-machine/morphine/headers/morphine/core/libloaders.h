//
// Created by whyiskra on 30.12.23.
//

#pragma once

#include "morphine/platform.h"

void mlib_base_loader(morphine_state_t);
void mlib_value_loader(morphine_state_t);
void mlib_gc_loader(morphine_state_t);
void mlib_fiber_loader(morphine_state_t);
void mlib_math_loader(morphine_state_t);
void mlib_string_loader(morphine_state_t);
void mlib_table_loader(morphine_state_t);
void mlib_registry_loader(morphine_state_t);
