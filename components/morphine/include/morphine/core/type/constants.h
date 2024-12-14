//
// Created by why-iskra on 14.12.2024.
//

#pragma once

#define mtype(n, s)                static const char *const MTYPE_##n = #s;
#define mspec_type_object(i, n, s) mtype(n, s)
#define mspec_type_value(i, n, s)  mtype(n, s)

#include "specification.h"

#undef mtype
#undef mspec_type_object
#undef mspec_type_value
