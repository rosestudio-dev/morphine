//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/string.h"
#include "morphine/algorithm/hash.h"
#include "morphine/core/sso.h"
#include "morphine/core/throw.h"
#include "morphine/gc/allocator.h"
#include "morphine/utils/compare.h"
#include "morphine/utils/overflow.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static_assert(MPARAM_SSO_MAX_LEN <= (mm_typemax(ml_size) - 1), "sso max len is too large");

static struct string *create(morphine_instance_t I, ml_size size, char **buffer) {
    size_t raw_size = mm_overflow_opc_add((size_t) size, 1, goto error);
    raw_size = mm_overflow_opc_mul(raw_size, (size_t) sizeof(char), goto error);

    size_t alloc_size = mm_overflow_opc_add(raw_size, (size_t) sizeof(struct string), goto error);
    struct string *result = allocI_uni(I, NULL, alloc_size);

    char *str_p = ((void *) result) + sizeof(struct string);

    (*result) = (struct string) {
        .size = size,
        .chars = str_p,
        .hash.calculated = false,
        .hash.value = 0,
    };

    memset(str_p, 0, raw_size);

    if (mm_unlikely(buffer != NULL)) {
        (*buffer) = str_p;
    }

    objectI_init(I, objectI_cast(result), OBJ_TYPE_STRING);

    return result;
error:
    throwI_error(I, "string size too big");
}

struct string *stringI_createm(morphine_instance_t I, const char *str, ml_size size) {
    {
        struct string *sso_string = ssoI_get(I, str, size);
        if (sso_string != NULL) {
            return sso_string;
        }
    }

    char *buffer = NULL;
    struct string *result = create(I, size, &buffer);
    memcpy(buffer, str, ((size_t) size) * sizeof(char));

    ssoI_rec(I, result);

    return result;
}

struct string *stringI_createn(morphine_instance_t I, const char *str, size_t size) {
    ml_size msize = mm_overflow_opc_cast(ml_size, size, throwI_error(I, "string size is too large"));
    return stringI_createm(I, str, msize);
}

struct string *stringI_createva(morphine_instance_t I, const char *str, va_list args) {
    va_list temp;
    va_copy(temp, args);
    int rawsize = vsnprintf(NULL, 0, str, temp);
    va_end(temp);

    ml_size size = mm_overflow_opc_cast(ml_size, rawsize, throwI_error(I, "unable to parse cformat"));
    if (size <= MPARAM_SSO_MAX_LEN) {
        char buffer[((size_t) MPARAM_SSO_MAX_LEN) + 1];
        memset(buffer, 0, ((size_t) MPARAM_SSO_MAX_LEN) + 1);

        va_copy(temp, args);
        vsnprintf(buffer, ((size_t) size) + 1, str, temp);
        va_end(temp);

        struct string *sso_string = ssoI_get(I, buffer, size);
        if (sso_string != NULL) {
            return sso_string;
        }
    }

    char *buffer = NULL;
    struct string *result = create(I, size, &buffer);

    va_copy(temp, args);
    vsnprintf(buffer, ((size_t) size) + 1, str, temp);
    va_end(temp);

    ssoI_rec(I, result);

    return result;
}

struct string *stringI_create(morphine_instance_t I, const char *str) {
    return stringI_createn(I, str, strlen(str));
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

struct string *stringI_get(morphine_instance_t I, struct string *string, ml_size index) {
    if (string == NULL) {
        throwI_error(I, "string is null");
    }

    if (index >= string->size) {
        throwI_error(I, "string index out of bounce");
    }

    return stringI_createn(I, string->chars + index, 1);
}

struct string *stringI_concat(morphine_instance_t I, struct string *a, struct string *b) {
    if (a == NULL || b == NULL) {
        throwI_error(I, "string is null");
    }

    ml_size size = mm_overflow_opc_add(a->size, b->size, throwI_error(I, "too big concat string length"));
    if (size <= MPARAM_SSO_MAX_LEN) {
        char buffer[MPARAM_SSO_MAX_LEN];
        memcpy(buffer, a->chars, ((size_t) a->size) * sizeof(char));
        memcpy(buffer + a->size, b->chars, ((size_t) b->size) * sizeof(char));

        struct string *sso_string = ssoI_get(I, buffer, size);
        if (sso_string != NULL) {
            return sso_string;
        }
    }

    char *buffer;
    struct string *result = create(I, size, &buffer);
    memcpy(buffer, a->chars, ((size_t) a->size) * sizeof(char));
    memcpy(buffer + a->size, b->chars, ((size_t) b->size) * sizeof(char));

    ssoI_rec(I, result);

    return result;
}

int stringI_compare(morphine_instance_t I, struct string *a, struct string *b) {
    if (a == NULL || b == NULL) {
        throwI_error(I, "string is null");
    }

    return arrcmp(I, a->chars, b->chars, a->size, b->size, sizeof(char));
}

int stringI_cstr_compare(morphine_instance_t I, struct string *a, const char *b) {
    if (a == NULL || b == NULL) {
        throwI_error(I, "string is null");
    }

    size_t b_size = strlen(b);
    return arrcmp(I, a->chars, b, a->size, b_size, sizeof(char));
}

bool stringI_is_cstr_compatible(morphine_instance_t I, struct string *string) {
    if (string == NULL) {
        throwI_error(I, "string is null");
    }

    if (string->is_cstr_compatible.calculated) {
        return string->is_cstr_compatible.value;
    }

    bool is_compatible = true;
    for (ml_size i = 0; i < string->size; i++) {
        if (string->chars[i] == '\0') {
            is_compatible = false;
            break;
        }
    }

    string->is_cstr_compatible.calculated = true;
    string->is_cstr_compatible.value = is_compatible;

    return is_compatible;
}

ml_hash stringI_hash(morphine_instance_t I, struct string *string) {
    if (string == NULL) {
        throwI_error(I, "string is null");
    }

    if (mm_likely(string->hash.calculated)) {
        return string->hash.value;
    }

    ml_hash result = calchash((const uint8_t *) string->chars, ((size_t) string->size) * sizeof(char));

    string->hash.calculated = true;
    string->hash.value = result;

    return result;
}
