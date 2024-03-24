//
// Created by whyiskra on 3/23/24.
//

#pragma once

#include "mark.h"
#include "morphine/platform.h"

void gcf_internal_record(morphine_instance_t);

void gcf_prepare(morphine_instance_t);
bool gcf_increment(morphine_instance_t I);
bool gcf_finalize(morphine_instance_t I);
void gcf_sweep(morphine_instance_t I);
