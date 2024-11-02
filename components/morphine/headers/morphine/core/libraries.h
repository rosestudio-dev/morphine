//
// Created by why-iskra on 17.06.2024.
//

#pragma once

#include "morphine/platform.h"

struct library_instance {
    morphine_library_init_t init;
    struct string *name;
    struct table *table;
};

struct libraries {
    size_t allocated;
    size_t size;
    struct library_instance *array;
};

struct libraries librariesI_prototype(void);
void librariesI_free(morphine_instance_t, struct libraries *);

void librariesI_load(morphine_instance_t, morphine_library_t);
struct value librariesI_get(morphine_coroutine_t, const char *);
