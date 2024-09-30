//
// Created by whyiskra on 16.12.23.
//

#include "morphine/object/string.h"
#include "morphine/core/throw.h"
#include "morphine/core/sso.h"
#include "morphine/gc/allocator.h"
#include "morphine/utils/overflow.h"
#include "morphine/params.h"
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

    size_t raw_size = ((size + 1) * sizeof(char));
    size_t alloc_size = sizeof(struct string) + raw_size;
    struct string *result = allocI_uni(I, NULL, alloc_size);

    char *str_p = ((void *) result) + sizeof(struct string);

    (*result) = (struct string) {
        .size = (ml_size) size,
        .chars = str_p,
        .hash.calculated = false,
        .hash.value = 0
    };

    memset(str_p, 0, raw_size);

    if (unlikely(buffer != NULL)) {
        (*buffer) = str_p;
    }

    objectI_init(I, objectI_cast(result), OBJ_TYPE_STRING);

    return result;
}

struct string *stringI_createn(morphine_instance_t I, size_t size, const char *str) {
    {
        struct string *sso_string = ssoI_get(I, str, size);
        if (sso_string != NULL) {
            return sso_string;
        }
    }

    char *buffer = NULL;
    struct string *result = create(I, size, &buffer);
    memcpy(buffer, str, size * sizeof(char));

    ssoI_rec(I, result);

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

    if (size <= MPARAM_SSO_MAX_LEN) {
        char buffer[MPARAM_SSO_MAX_LEN];
        va_copy(temp, args);
        vsnprintf(buffer, size + 1, str, temp);
        va_end(temp);

        struct string *sso_string = ssoI_get(I, buffer, size);
        if (sso_string != NULL) {
            return sso_string;
        }
    }

    char *buffer = NULL;
    struct string *result = create(I, size, &buffer);

    va_copy(temp, args);
    vsnprintf(buffer, size + 1, str, temp);
    va_end(temp);

    ssoI_rec(I, result);

    return result;
}

struct string *stringI_create(morphine_instance_t I, const char *str) {
    return stringI_createn(I, strlen(str), str);
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

    return stringI_createn(I, 1, string->chars + index);
}

struct string *stringI_concat(morphine_instance_t I, struct string *a, struct string *b) {
    if (a == NULL || b == NULL) {
        throwI_error(I, "string is null");
    }

    overflow_add(a->size, b->size, MLIMIT_SIZE_MAX) {
        throwI_error(I, "too big concat string length");
    }

    size_t size = a->size + b->size;
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

    if (a->size > b->size) {
        return -1;
    } else if (a->size < b->size) {
        return 1;
    }

    return memcmp(a->chars, b->chars, ((size_t) a->size) * sizeof(char));
}

int stringI_cstr_compare(morphine_instance_t I, struct string *a, const char *b) {
    if (a == NULL || b == NULL) {
        throwI_error(I, "string is null");
    }

    size_t b_size = strlen(b);
    if (a->size > b_size) {
        return -1;
    } else if (a->size < b_size) {
        return 1;
    }

    return memcmp(a->chars, b, ((size_t) a->size) * sizeof(char));
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

uint64_t stringI_rawhash(size_t size, const char *str) {
    return hash(size, str);
}

struct value stringI_iterator_first(morphine_instance_t I, struct string *string, bool *has) {
    if (string == NULL) {
        throwI_error(I, "string is null");
    }

    if (has != NULL) {
        (*has) = string->size > 0;
    }

    if (string->size == 0) {
        return valueI_nil;
    }

    return valueI_integer(0);
}

struct pair stringI_iterator_next(
    morphine_instance_t I,
    struct string *string,
    struct value *key,
    bool *next
) {
    if (string == NULL) {
        throwI_error(I, "string is null");
    }

    if (key == NULL) {
        if (next != NULL) {
            (*next) = false;
        }

        return valueI_pair(valueI_nil, valueI_nil);
    }

    ml_size index = valueI_integer2index(I, valueI_as_integer_or_error(I, *key));

    if (index >= string->size) {
        if (next != NULL) {
            (*next) = false;
        }

        (*key) = valueI_nil;
        return valueI_pair(valueI_nil, valueI_nil);
    }

    bool has_next = (index + 1) < string->size;
    if (next != NULL) {
        (*next) = has_next;
    }

    if (has_next) {
        (*key) = valueI_size(valueI_csize2index(I, index + 1));
    } else {
        (*key) = valueI_nil;
    }

    struct string *char_string = stringI_get(I, string, index);
    return valueI_pair(valueI_size(index), valueI_object(char_string));
}
