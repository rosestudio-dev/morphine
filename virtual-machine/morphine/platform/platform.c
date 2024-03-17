//
// Created by whyiskra on 30.12.23.
//

#include "morphine/platform.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define DIGITS      "0123456789abcdefghijklmnopqrstuvwxyz"
#define DIGITS_SIZE (sizeof(DIGITS) - 1)

static inline int32_t todigit(const char c, uint8_t base) {
    int lower = tolower(c);
    for (int32_t i = 0; i < base; i++) {
        if (DIGITS[i] == lower) {
            return i;
        }
    }

    return -1;
}

bool platformI_string2integer(const char *string, morphine_integer_t *container, uint8_t base) {
    if ((base < 2) || (base > DIGITS_SIZE) || (string == NULL)) {
        return false;
    }

    size_t index = 0;

    bool negative = false;
    morphine_integer_t limit = -MORPHINE_INTEGER_MAX;
    morphine_integer_t result = 0;

    char firstchar = *string;
    if (todigit(firstchar, base) < 0) {
        if (firstchar == '-') {
            negative = true;
            limit = MORPHINE_INTEGER_MIN;
        } else if (firstchar != '+') {
            return false;
        }

        if (*(string + 1) == '\0') {
            return false;
        }

        index++;
        string++;
    }

    morphine_integer_t multmin = limit / base;

    while (*string != '\0') {
        if (index > 100) {
            return false;
        }

        int32_t digit = todigit(*string, base);

        if (digit < 0 || result < multmin) {
            return false;
        }

        result *= base;

        if (result < limit + digit) {
            return false;
        }

        result -= digit;

        index++;
        string++;
    }

    *container = negative ? result : -result;

    return true;
}

bool platformI_string2decimal(const char *string, morphine_decimal_t *container) {
    size_t len = 0;
    {
        const char *temp = string;
        while (*temp != '\0') {
            if (len > 100) {
                return false;
            }

            char c = *temp;

            if (!isdigit(c) && c != '.' && c != '+' && c != '-') {
                return false;
            }

            len++;
            temp++;
        }
    }

    char *end;
    double num = morphine_decimal_tostrc(string, &end);

    if ((string + len != end)) {
        return false;
    }

    *container = (morphine_decimal_t) num;

    return true;
}
