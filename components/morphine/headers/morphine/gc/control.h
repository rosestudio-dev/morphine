//
// Created by whyiskra on 3/23/24.
//

#pragma once

#include "morphine/platform.h"

void gcI_change_threshold(morphine_instance_t, size_t);
void gcI_change_limit(morphine_instance_t, size_t);
void gcI_change_grow(morphine_instance_t, uint16_t);
void gcI_change_deal(morphine_instance_t, uint16_t);
void gcI_change_pause(morphine_instance_t, uint8_t);
void gcI_change_cache_callinfo(morphine_instance_t, size_t);

void gcI_enable(morphine_instance_t);
void gcI_disable(morphine_instance_t);
void gcI_force(morphine_instance_t);

void gcI_work(morphine_instance_t, size_t reserved);
void gcI_full(morphine_instance_t, size_t reserved);
