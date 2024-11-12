//
// Created by why-iskra on 31.03.2024.
//

#pragma once

#include "morphine/platform.h"

MORPHINE_AUX bool maux_sharedstorage_get(morphine_coroutine_t, const char *, const char *);
MORPHINE_AUX void maux_sharedstorage_getoe(morphine_coroutine_t, const char *, const char *);
MORPHINE_AUX void maux_sharedstorage_set(morphine_coroutine_t, const char *, const char *);
MORPHINE_AUX bool maux_sharedstorage_remove(morphine_coroutine_t, const char *, const char *);
MORPHINE_AUX void maux_sharedstorage_removeoe(morphine_coroutine_t, const char *, const char *);
