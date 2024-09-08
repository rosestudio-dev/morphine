//
// Created by why-iskra on 31.03.2024.
//

#pragma once

#include "morphine/platform.h"

MORPHINE_AUX bool maux_localstorage_get(morphine_coroutine_t, const char *);
MORPHINE_AUX void maux_localstorage_getoe(morphine_coroutine_t, const char *);
MORPHINE_AUX void maux_localstorage_set(morphine_coroutine_t, const char *);
MORPHINE_AUX bool maux_localstorage_remove(morphine_coroutine_t, const char *);
MORPHINE_AUX void maux_localstorage_removeoe(morphine_coroutine_t, const char *);
