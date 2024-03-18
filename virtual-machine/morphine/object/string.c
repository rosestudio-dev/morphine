//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/string.h"
#include "morphine/core/allocator.h"
#include "morphine/core/throw.h"
#include <stdio.h>
#include <string.h>

static struct string *create(morphine_instance_t I, size_t size, char **buffer) {
    size_t alloc_size = sizeof(struct string) + sizeof(char) * (size + 1);

    struct string *result = allocI_uni(I, NULL, alloc_size);

    char *str_p = ((void *) result) + sizeof(struct string);

    (*result) = (struct string) {
        .size = size,
        .chars = str_p
    };

    for (size_t i = 0; i < (size + 1); i++) {
        str_p[i] = '\0';
    }

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
        throwI_message_panic(I, NULL, "Cannot parse cformat");
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

size_t stringI_allocated_size(struct string *string) {
    return sizeof(struct string) + sizeof(char) * (string->size + 1);
}
