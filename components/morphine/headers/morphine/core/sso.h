//
// Created by why-iskra on 17.09.2024.
//

#pragma once

#include "morphine/object/string.h"
#include "morphine/params.h"

struct sso {
    struct string *table[MPARAM_SSO_HASHTABLE_ROWS][MPARAM_SSO_HASHTABLE_COLS];
};

struct sso ssoI_prototype(void);

struct string *ssoI_get(morphine_instance_t, const char *, ml_size);
void ssoI_rec(morphine_instance_t, struct string *);
