//
// Created by why-iskra on 31.03.2024.
//

#pragma once

#include "morphine/platform.h"

MORPHINE_AUX bool maux_table_access(morphine_coroutine_t, const char *);
MORPHINE_AUX bool maux_table_has(morphine_coroutine_t, const char *);
MORPHINE_AUX bool maux_table_get(morphine_coroutine_t, const char *);
MORPHINE_AUX void maux_table_getoe(morphine_coroutine_t, const char *);
MORPHINE_AUX void maux_table_set(morphine_coroutine_t, const char *);
MORPHINE_AUX bool maux_table_remove(morphine_coroutine_t, const char *);
MORPHINE_AUX void maux_table_removeoe(morphine_coroutine_t, const char *);
