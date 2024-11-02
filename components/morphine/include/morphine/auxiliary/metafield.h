//
// Created by why-iskra on 02.11.2024.
//

#pragma once

#include "morphine/platform.h"
#include "morphine/misc/metatable/type.h"

MORPHINE_AUX const char *maux_metafield_name(morphine_coroutine_t, morphine_metatable_field_t);
MORPHINE_AUX morphine_metatable_field_t maux_metafield_from_name(morphine_coroutine_t, const char *);
