//
// Created by why-iskra on 17.09.2024.
//

#include <string.h>
#include "morphine/core/sso.h"
#include "morphine/core/instance.h"
#include "morphine/algorithm/hash.h"

struct sso ssoI_prototype(void) {
    struct sso result;
    for (size_t r = 0; r < MPARAM_SSO_HASHTABLE_ROWS; r++) {
        for (size_t c = 0; c < MPARAM_SSO_HASHTABLE_COLS; c++) {
            result.table[r][c] = NULL;
        }
    }

    return result;
}

struct string *ssoI_get(morphine_instance_t I, const char *str, size_t size) {
    if (size > MPARAM_SSO_MAX_LEN) {
        return NULL;
    }

#ifdef MORPHINE_ENABLE_SSO
    ml_hash hash = calchash(size, (const uint8_t *) str);

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
#endif

    return NULL;
}

void ssoI_rec(morphine_instance_t I, struct string *string) {
    if (string->size > MPARAM_SSO_MAX_LEN) {
        return;
    }

#ifdef MORPHINE_ENABLE_SSO
    ml_hash hash = stringI_hash(I, string);

    struct string **bucket = I->sso.table[hash % MPARAM_SSO_HASHTABLE_ROWS];
    for (size_t i = 1; i < MPARAM_SSO_HASHTABLE_COLS; i++) {
        size_t index = MPARAM_SSO_HASHTABLE_COLS - i - 1;
        bucket[index + 1] = bucket[index];
    }

    bucket[0] = string;
#else
    (void) I;
#endif
}
