//
// Created by why-iskra on 31.03.2024.
//

#pragma once

#include "morphine/platform.h"

MORPHINE_AUX bool maux_registry_get(morphine_coroutine_t, const char *);
MORPHINE_AUX void maux_registry_getoe(morphine_coroutine_t, const char *);
MORPHINE_AUX void maux_registry_set(morphine_coroutine_t, const char *);
MORPHINE_AUX bool maux_registry_remove(morphine_coroutine_t, const char *);
MORPHINE_AUX void maux_registry_removeoe(morphine_coroutine_t, const char *);
