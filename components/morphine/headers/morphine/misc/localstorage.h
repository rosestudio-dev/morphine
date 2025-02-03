//
// Created by whyiskra on 07.01.24.
//

#pragma once

#include "morphine/object/coroutine.h"

void localstorageI_set(morphine_coroutine_t, struct value key, struct value value);
struct value localstorageI_get(morphine_coroutine_t, struct value key, bool *has);
struct value localstorageI_remove(morphine_coroutine_t, struct value key, bool *has);

void localstorageI_clear(morphine_instance_t, struct callframe *);
