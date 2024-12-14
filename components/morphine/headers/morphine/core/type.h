//
// Created by whyiskra on 3/15/24.
//

#pragma once

#include "morphine/platform.h"

enum obj_type {
#define mspec_type_object(i, n, s) OBJ_TYPE_##n = (i),
#define mspec_type_value(i, n, s)

#include "morphine/core/type/specification.h"

#undef mspec_type_object
#undef mspec_type_value
};

enum value_type {
#define mspec_type_object(i, n, s) VALUE_TYPE_##n = (i),
#define mspec_type_value(i, n, s)  VALUE_TYPE_##n = (i),

#include "morphine/core/type/specification.h"

#undef mspec_type_object
#undef mspec_type_value
};

const char *typeI_tostring(morphine_instance_t, enum value_type);
enum value_type typeI_fromstring(morphine_instance_t, const char *);
bool typeI_isobj(enum value_type);
