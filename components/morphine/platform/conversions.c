//
// Created by whyiskra on 30.12.23.
//

#include "morphine/platform/conversions.h"
#include <ctype.h>
#include <stdio.h>

bool platformI_string2integer(const char *string, ml_integer *container) {
    size_t len = 0;
    {
        const char *temp = string;
        while (*temp != '\0') {
            if (len > 64) {
                return false;
            }

            char c = *temp;

            if (!isdigit(c) && c != '+' && c != '-') {
                return false;
            }

            len++;
            temp++;
        }
    }

    ml_integer num = 0;
    int result = sscanf(string, "%"MLIMIT_INTEGER_SC, &num);

    *container = num;
    return result == 1;
}

bool platformI_string2decimal(const char *string, ml_decimal *container) {
    size_t len = 0;
    {
        const char *temp = string;
        while (*temp != '\0') {
            if (len > 64) {
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

    ml_decimal num = 0;
    int result = sscanf(string, "%"MLIMIT_DECIMAL_SC, &num);

    *container = num;
    return result == 1;
}
