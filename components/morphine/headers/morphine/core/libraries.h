//
// Created by why-iskra on 17.06.2024.
//

#pragma once

#include "morphine/platform.h"

struct library {
    morphine_library_t *L;
    struct table *table;
};

struct libraries {
    size_t allocated;
    size_t size;
    struct library *array;
};

struct libraries librariesI_prototype(void);
void librariesI_free(morphine_instance_t, struct libraries *);

void librariesI_load(morphine_instance_t, morphine_library_t *);
struct value librariesI_get(morphine_instance_t, const char *, bool reload);
