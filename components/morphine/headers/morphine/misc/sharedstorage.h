//
// Created by whyiskra on 07.01.24.
//

#pragma once

#include "morphine/core/value.h"

bool sharedstorageI_define(morphine_instance_t, const char *);
void sharedstorageI_set(morphine_instance_t, const char *, struct value key, struct value value);
struct value sharedstorageI_get(morphine_instance_t, const char *, struct value key, bool *has);
struct value sharedstorageI_remove(morphine_instance_t, const char *, struct value key, bool *has);
void sharedstorageI_clear(morphine_instance_t, const char *);
