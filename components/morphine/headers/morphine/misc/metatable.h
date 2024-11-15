//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"
#include "morphine/misc/metatable/type.h"

void metatableI_set(morphine_instance_t, struct value, struct table *);
struct value metatableI_get(morphine_instance_t, struct value);

bool metatableI_builtin_test(morphine_instance_t, struct value, morphine_metatable_field_t, struct value *);
bool metatableI_test(morphine_instance_t, struct value, struct string *, struct value *);

const char *metatableI_field2string(morphine_instance_t, morphine_metatable_field_t);
morphine_metatable_field_t metatableI_string2field(morphine_instance_t, const char *name);
