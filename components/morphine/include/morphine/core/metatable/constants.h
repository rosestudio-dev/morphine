//
// Created by why-iskra on 14.12.2024.
//

#pragma once

#define mstr_mf(s) static const char *const mstr_mf_##s = #s;
#define mspec_metatable_field(n, s) mstr_mf(s)

#include "specification.h"

#undef mstr_mf
#undef mspec_metatable_field
