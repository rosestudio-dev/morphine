//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include <stdarg.h>
#include "morphine/gc/object.h"

struct string {
    struct object header;

    ml_size size;
    const char *chars;

    struct {
        bool calculated;
        uint64_t value;
    } hash;
};

struct string *stringI_createn(morphine_instance_t, size_t, const char *);
struct string *stringI_createva(morphine_instance_t, const char *, va_list);
struct string *stringI_createf(morphine_instance_t, const char *, ...);
struct string *stringI_create(morphine_instance_t, const char *);

void stringI_free(morphine_instance_t, struct string *);

struct string *stringI_get(morphine_instance_t, struct string *, ml_size);

struct string *stringI_concat(morphine_instance_t, struct string *, struct string *);
uint64_t stringI_hash(morphine_instance_t, struct string *);
uint64_t stringI_rawhash(size_t, const char *);

struct value stringI_iterator_first(morphine_instance_t, struct string *, bool *has);
struct pair stringI_iterator_next(morphine_instance_t, struct string *, struct value *key, bool *next);
