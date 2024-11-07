//
// Created by whyiskra on 3/23/24.
//

#pragma once

#include "morphine/platform.h"

#define gcI_finalize_need(I) ((I)->G.pools.finalize != NULL || (I)->G.finalizer.coroutine != NULL)

void gcI_init_finalizer(morphine_instance_t);
void gcI_finalize(morphine_instance_t);
