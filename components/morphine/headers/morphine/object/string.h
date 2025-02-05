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
        ml_hash value;
    } hash;

    struct {
        bool calculated;
        bool value;
    } is_cstr_compatible;
};

struct string *stringI_createm(morphine_instance_t, const char *, ml_size);
struct string *stringI_createn(morphine_instance_t, const char *, size_t);
struct string *stringI_createva(morphine_instance_t, const char *, va_list);
mattr_printf(2, 3) struct string *stringI_createf(morphine_instance_t, const char *, ...);
struct string *stringI_create(morphine_instance_t, const char *);
void stringI_free(morphine_instance_t, struct string *);

struct string *stringI_get(morphine_instance_t, struct string *, ml_size);
struct string *stringI_concat(morphine_instance_t, struct string *, struct string *);
bool stringI_is_cstr_compatible(morphine_instance_t, struct string *);
int stringI_compare(morphine_instance_t, struct string *, struct string *);
int stringI_cstr_compare(morphine_instance_t, struct string *, const char *);
ml_hash stringI_hash(morphine_instance_t, struct string *);

struct value stringI_iterator_first(morphine_instance_t, struct string *, bool *has);
struct pair stringI_iterator_next(morphine_instance_t, struct string *, struct value *key, bool *next);
