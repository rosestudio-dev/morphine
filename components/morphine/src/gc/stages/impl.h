//
// Created by why-iskra on 25.03.2024.
//

#pragma once

#include "morphine/platform.h"

void gcstageI_prepare(morphine_instance_t);
bool gcstageI_increment(morphine_instance_t, size_t debt);
void gcstageI_resolve(morphine_instance_t, bool emergency);
bool gcstageI_sweep(morphine_instance_t, size_t debt);