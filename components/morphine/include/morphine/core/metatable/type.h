//
// Created by why-iskra on 31.10.2024.
//

#pragma once

#define MORPHINE_METAFIELDS_START (MTYPE_METAFIELD_TYPE)
#define MORPHINE_METAFIELDS_COUNT (MTYPE_METAFIELD_GC + 1)
#define MORPHINE_METAFIELD_PREFIX "_mf_"

typedef enum {
#define mspec_metatable_field(n, s) MTYPE_METAFIELD_##n,

#include "specification.h"

#undef mspec_metatable_field
} mtype_metafield_t;
