//
// Created by why-iskra on 14.12.2024.
//

#pragma once

#define mstr_type(s) static const char *const mstr_type_##s = #s;
#define mspec_type_object(i, n, s) mstr_type(s)
#define mspec_type_value(i, n, s)  mstr_type(s)

#include "specification.h"

#undef mstr_type
#undef mspec_type_object
#undef mspec_type_value
