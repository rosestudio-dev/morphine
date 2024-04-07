//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/string.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include <stdio.h>
#include <string.h>

static struct string *create(morphine_instance_t I, size_t size, char **buffer) {
    bool check1 = size > MLIMIT_SIZE_MAX - 1;
    bool check2 = (size + 1) > SIZE_MAX / sizeof(char);
    bool check3 = (size + 1) * sizeof(char) > SIZE_MAX - sizeof(struct string);
    if (check1 || check2 || check3) {
        throwI_error(I, "String size too big");
    }

    size_t alloc_size = sizeof(struct string) + ((size + 1) * sizeof(char));
    struct string *result = allocI_uni(I, NULL, alloc_size);

    char *str_p = ((void *) result) + sizeof(struct string);

    (*result) = (struct string) {
        .size = (ml_size) size,
        .chars = str_p
    };

    memset(str_p, 0, (size + 1) * sizeof(char));

    if (buffer != NULL) {
        (*buffer) = str_p;
    }

    objectI_init(I, objectI_cast(result), OBJ_TYPE_STRING);

    return result;
}

struct string *stringI_createn(morphine_instance_t I, size_t size, char **buffer) {
    return create(I, size, buffer);
}

struct string *stringI_create(morphine_instance_t I, const char *str) {
    char *buffer = NULL;
    struct string *result = create(I, strlen(str), &buffer);
    strcpy(buffer, str);

    return result;
}

struct string *stringI_createva(morphine_instance_t I, const char *str, va_list args) {
    va_list temp;
    va_copy(temp, args);
    int rawsize = vsnprintf(NULL, 0, str, temp);
    va_end(temp);

    if (rawsize < 0) {
        throwI_error(I, "Cannot parse cformat");
    }

    size_t size = (size_t) rawsize;

    char *buffer = NULL;
    struct string *result = create(I, size, &buffer);

    va_copy(temp, args);
    vsnprintf(buffer, size + 1, str, temp);
    va_end(temp);

    return result;
}

struct string *stringI_createf(morphine_instance_t I, const char *str, ...) {
    va_list args;
    va_start(args, str);
    struct string *result = stringI_createva(I, str, args);
    va_end(args);

    return result;
}

void stringI_free(morphine_instance_t I, struct string *string) {
    allocI_free(I, string);
}

struct string *stringI_concat(morphine_instance_t I, struct string *a, struct string *b) {
    if (a == NULL || b == NULL) {
        throwI_error(I, "Reference is null");
    }

    char *buffer;
    struct string *result = create(I, a->size + b->size, &buffer);
    memcpy(buffer, a->chars, a->size);
    memcpy(buffer + a->size, b->chars, b->size);

    return result;
}
