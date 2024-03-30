//
// Created by whyiskra on 07.01.24.
//

#pragma once

#include "value.h"

void registryI_set_key(morphine_instance_t, morphine_state_t, struct value callable, struct value key);

void registryI_set(morphine_state_t, struct value key, struct value value);
struct value registryI_get(morphine_state_t, struct value key, bool *has);
void registryI_clear(morphine_state_t);
