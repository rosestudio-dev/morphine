//
// Created by why-iskra on 02.11.2024.
//

#pragma once

#include "morphine/platform.h"
#include "morphine/misc/metatable/type.h"

MORPHINE_AUX const char *maux_metafield_name(morphine_coroutine_t, mtype_metafield_t);
MORPHINE_AUX mtype_metafield_t maux_metafield_from_name(morphine_coroutine_t, const char *);
