//
// Created by whyiskra on 19.12.23.
//

#pragma once

#include "morphine/platform.h"

void *allocI_uni(morphine_instance_t, void *p, size_t nsize);
void allocI_free(morphine_instance_t, void *p);
