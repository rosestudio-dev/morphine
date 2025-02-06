//
// Created by why-iskra on 17.06.2024.
//

#pragma once

#include "morphine/platform.h"

struct library {
    mfunc_native_t init;
    struct string *name;
    struct table *table;

    struct library *prev;
};

struct libraries {
    struct library *list;
};

struct libraries librariesI_prototype(void);
void librariesI_free(morphine_instance_t, struct libraries *);

void librariesI_load(morphine_instance_t, morphine_library_t);
struct value librariesI_get(morphine_coroutine_t, const char *);
