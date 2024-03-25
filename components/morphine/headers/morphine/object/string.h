//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include <stdarg.h>
#include "morphine/core/object.h"

struct string {
    struct object header;

    size_t size;
    const char *chars;
};

struct string *stringI_createva(morphine_instance_t, const char *str, va_list args);
struct string *stringI_createf(morphine_instance_t, const char *, ...);
struct string *stringI_createn(morphine_instance_t, size_t size, char **buffer);
struct string *stringI_create(morphine_instance_t, const char *);

void stringI_free(morphine_instance_t, struct string *);

size_t stringI_allocated_size(struct string *);

struct string *stringI_concat(morphine_instance_t, struct string *, struct string *);
