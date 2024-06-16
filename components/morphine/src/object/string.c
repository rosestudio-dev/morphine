//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/string.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/utils/overflow.h"
#include <string.h>
#include <stdio.h>

static inline uint64_t hash(size_t size, const char *str) {
    uint64_t h = 0;
    for (ml_size i = 0; i < size; i++) {
        h = 31 * h + (uint64_t) str[i];
    }

    return h;
}

static struct string *create(morphine_instance_t I, size_t size, char **buffer) {
    bool check1 = overflow_condition_add(size, 1, MLIMIT_SIZE_MAX);
    bool check2 = overflow_condition_mul(size + 1, sizeof(char), SIZE_MAX);
    bool check3 = overflow_condition_add(size + 1 * sizeof(char), sizeof(struct string), SIZE_MAX);
    if (check1 || check2 || check3) {
        throwI_error(I, "string size too big");
    }

    size_t alloc_size = sizeof(struct string) + ((size + 1) * sizeof(char));
    struct string *result = allocI_uni(I, NULL, alloc_size);

    char *str_p = ((void *) result) + sizeof(struct string);

    (*result) = (struct string) {
        .size = (ml_size) size,
        .chars = str_p,
        .hash.calculated = false,
        .hash.value = 0
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
    size_t size = strlen(str);
    char *buffer = NULL;
    struct string *result = create(I, size, &buffer);
    strcpy(buffer, str);

    return result;
}

struct string *stringI_createva(morphine_instance_t I, const char *str, va_list args) {
    va_list temp;
    va_copy(temp, args);
    int rawsize = vsnprintf(NULL, 0, str, temp);
    va_end(temp);

    if (rawsize < 0) {
        throwI_error(I, "cannot parse cformat");
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
        throwI_error(I, "string is null");
    }

    overflow_add(a->size, b->size, MLIMIT_SIZE_MAX) {
        throwI_error(I, "too big concat string length");
    }

    char *buffer;
    struct string *result = create(I, a->size + b->size, &buffer);
    memcpy(buffer, a->chars, a->size);
    memcpy(buffer + a->size, b->chars, b->size);

    return result;
}

uint64_t stringI_hash(morphine_instance_t I, struct string *string) {
    if (string == NULL) {
        throwI_error(I, "string is null");
    }

    if (likely(string->hash.calculated)) {
        return string->hash.value;
    }

    uint64_t result = hash(string->size, string->chars);

    string->hash.calculated = true;
    string->hash.value = result;

    return result;
}
