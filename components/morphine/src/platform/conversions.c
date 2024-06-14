//
// Created by whyiskra on 30.12.23.
//

#include "morphine/platform/conversions.h"
#include "morphine/utils/overflow.h"
#include <ctype.h>
#include <limits.h>
#include <stdio.h>

#define INTEGER_LOOP_MAX_ITERATIONS ((sizeof(ml_integer) * CHAR_BIT) + 1)
#define DECIMAL_LOOP_MAX_ITERATIONS ((sizeof(ml_decimal) * CHAR_BIT) + 2)
#define SIZE_LOOP_MAX_ITERATIONS    (sizeof(ml_size) * CHAR_BIT)

static ml_integer digit(char c, ml_integer base) {
    if (base < 2 || base > 36) {
        return -1;
    }

    if (isdigit(c)) {
        ml_integer n = ((ml_integer) c) - '0';

        if (n >= base || n < 0 || n >= 10) {
            return -1;
        }

        return n;
    }

    if (isalpha(c)) {
        ml_integer n = ((ml_integer) c) - (isupper(c) ? 'A' : 'a');

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
        overflow_signed_mul(num, ibase, MLIMIT_INTEGER_MIN, MLIMIT_INTEGER_MAX) {
            goto error;
        } else {
            num *= ibase;
        }

        bool minus_overflow = negative && overflow_condition_signed_sub(
            num, n, MLIMIT_INTEGER_MIN, MLIMIT_INTEGER_MAX
        );

        bool plus_overflow = !negative && overflow_condition_signed_add(
            num, n, MLIMIT_INTEGER_MIN, MLIMIT_INTEGER_MAX
        );

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

bool platformI_string2size(const char *string, ml_size *container, ml_size base) {
    if (string == NULL || string[0] == '\0') {
        goto error;
    }

    ml_size num = 0;
    for (size_t i = 0; i < SIZE_LOOP_MAX_ITERATIONS; i++) {
        char ch = string[i];

        if (ch == '\0') {
            if (i == 0) {
                goto error;
            }

            if (container != NULL) {
                *container = num;
            }

            return true;
        }

        ml_integer rn = digit(ch, base);
        if (rn < 0) {
            goto error;
        }

        ml_size n = (ml_size) rn;

        overflow_mul(num, base, MLIMIT_SIZE_MAX) {
            goto error;
        } else {
            num *= base;
        }

        overflow_add(num, n, MLIMIT_SIZE_MAX) {
            goto error;
        } else {
            num += n;
        }
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

        if (!isdigit(c) && c != '.' && c != '+' && c != '-') {
            goto error;
        }
    }

    if (len == 0) {
        goto error;
    }

    ml_decimal num = 0;
    int result = sscanf(string, "%"MLIMIT_DECIMAL_SC, &num);

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
