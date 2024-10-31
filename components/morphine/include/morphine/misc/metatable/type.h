//
// Created by why-iskra on 31.10.2024.
//

#pragma once

#define MORPHINE_METATABLE_FIELDS_START (MORPHINE_METAFIELD_TYPE)
#define MORPHINE_METATABLE_FIELDS_COUNT (MORPHINE_METAFIELD_GC + 1)

typedef enum {
#define mspec_metatable_field(n, s) MORPHINE_METAFIELD_##n,

#include "specification.h"

#undef mspec_metatable_field
} morphine_metatable_field_t;
