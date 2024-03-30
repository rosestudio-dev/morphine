//
// Created by why-iskra on 25.03.2024.
//

#pragma once

#include "morphine/platform.h"

void gcstageI_record(morphine_instance_t);

void gcstageI_prepare(morphine_instance_t);
bool gcstageI_increment(morphine_instance_t);
bool gcstageI_finalize(morphine_instance_t);
void gcstageI_sweep(morphine_instance_t);