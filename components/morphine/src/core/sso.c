//
// Created by why-iskra on 17.09.2024.
//

#include "morphine/core/sso.h"
#include "morphine/algorithm/hash.h"
#include "morphine/core/instance.h"
#include <string.h>

struct sso ssoI_prototype(void) {
    struct sso result;
    for (size_t r = 0; r < MPARAM_SSO_HASHTABLE_ROWS; r++) {
        for (size_t c = 0; c < MPARAM_SSO_HASHTABLE_COLS; c++) {
            result.table[r][c] = NULL;
        }
    }

    return result;
}

struct string *ssoI_get(morphine_instance_t I, const char *str, ml_size size) {
#ifdef MORPHINE_ENABLE_SSO
    if (size > MPARAM_SSO_MAX_LEN) {
        return NULL;
    }

    ml_hash hash = calchash((const uint8_t *) str, ((size_t) size) * sizeof(char));

    struct string **bucket = I->sso.table[hash % MPARAM_SSO_HASHTABLE_ROWS];
    for (size_t i = 0; i < MPARAM_SSO_HASHTABLE_COLS; i++) {
        struct string *string = bucket[i];

        if (string == NULL || string->size != size) {
            continue;
        }

        size_t memsize = string->size;
        if (memcmp(string->chars, str, memsize * sizeof(char)) == 0) {
            struct string *temp = bucket[0];
            bucket[0] = string;
            bucket[i] = temp;

            return string;
        }
    }
#else
    (void) I;
    (void) str;
    (void) size;
#endif

    return NULL;
}

void ssoI_rec(morphine_instance_t I, struct string *string) {
#ifdef MORPHINE_ENABLE_SSO
    if (string->size > MPARAM_SSO_MAX_LEN) {
        return;
    }

    ml_hash hash = stringI_hash(string);

    struct string **bucket = I->sso.table[hash % MPARAM_SSO_HASHTABLE_ROWS];
    for (size_t i = 1; i < MPARAM_SSO_HASHTABLE_COLS; i++) {
        size_t index = MPARAM_SSO_HASHTABLE_COLS - i - 1;
        bucket[index + 1] = bucket[index];
    }

    bucket[0] = string;
#else
    (void) I;
    (void) string;
#endif
}
