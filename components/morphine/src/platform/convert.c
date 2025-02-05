//
// Created by whyiskra on 30.12.23.
//

#include "morphine/platform/convert.h"
#include "morphine/utils/ctype.h"
#include "morphine/utils/overflow.h"
#include <limits.h>
#include <stdio.h>

#define INTEGER_LOOP_MAX_ITERATIONS ((sizeof(ml_integer) * CHAR_BIT) + 1)
#define DECIMAL_LOOP_MAX_ITERATIONS ((sizeof(ml_decimal) * CHAR_BIT) + 2)
#define SIZE_LOOP_MAX_ITERATIONS    (sizeof(ml_size) * CHAR_BIT)

static ml_integer digit(char c, ml_integer base) {
    if (base < 2 || base > 36) {
        return -1;
    }

    if (mm_ctype_isdigit(c)) {
        ml_integer n = ((ml_integer) c) - '0';

        if (n >= base || n < 0 || n >= 10) {
            return -1;
        }

        return n;
    }

    if (mm_ctype_isalpha(c)) {
        ml_integer n = ((ml_integer) c) - (mm_ctype_isupper(c) ? 'A' : 'a');

        if ((n + 10) >= base || n < 0 || n >= 26) {
            return -1;
        }

        return n + 10;
    }

    return -1;
}

bool platformI_string2integer(const char *string, ml_integer *container, ml_size base) {
    if (string == NULL || string[0] == '\0') {
        goto error;
    }

    char first = string[0];
    ml_integer num = 0;

    bool positive = first == '+';
    bool negative = first == '-';

    if (!positive && !negative) {
        num = digit(first, base);

        if (num < 0) {
            goto error;
        }
    }

    for (size_t i = 1; i < INTEGER_LOOP_MAX_ITERATIONS; i++) {
        char ch = string[i];

        if (ch == '\0') {
            if ((positive || negative) && i == 1) {
                goto error;
            }

            if (container != NULL) {
                *container = num;
            }

            return true;
        }

        ml_integer n = digit(ch, base);
        if (n < 0) {
            goto error;
        }

        ml_integer ibase = (ml_integer) base; // bypass 'type-limit' warning
        mm_overflow_mul(num, ibase) {
            goto error;
        }

        num *= ibase;

        bool minus_overflow = negative && mm_overflow_cond_sub(num, n);
        bool plus_overflow = !negative && mm_overflow_cond_add(num, n);

        if (plus_overflow || minus_overflow) {
            goto error;
        }

        num += negative ? -n : n;
    }

error:
    if (container != NULL) {
        *container = 0;
    }

    return false;
}

bool platformI_string2decimal(const char *string, ml_decimal *container) {
    if (string == NULL) {
        goto error;
    }

    size_t len = 0;
    for (; len < DECIMAL_LOOP_MAX_ITERATIONS; len++) {
        char c = string[len];

        if (c == '\0') {
            break;
        }

        if (!mm_ctype_isdigit(c) && c != '.' && c != '+' && c != '-') {
            goto error;
        }
    }

    if (len == 0) {
        goto error;
    }

    ml_decimal num = 0;
    int result = sscanf(string, "%" MLIMIT_DECIMAL_PR, &num);

    if (container != NULL) {
        *container = num;
    }

    return result == 1;

error:
    if (container != NULL) {
        *container = 0.0;
    }

    return false;
}
