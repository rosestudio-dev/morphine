//
// Created by why-iskra on 21.07.2024.
//

#pragma once

#include "morphine/platform.h"

struct usertype {
    const char *name;

    size_t allocate;
    mfunc_constructor_t constructor;
    mfunc_destructor_t destructor;
    struct table *metatable;

    struct usertype *prev;
};

struct usertypes {
    struct usertype *list;
};

struct usertypes usertypeI_prototype(void);
void usertypeI_free(morphine_instance_t, struct usertypes *);

void usertypeI_declare(
    morphine_instance_t,
    const char *name,
    size_t allocate,
    mfunc_constructor_t,
    mfunc_destructor_t,
    struct table *metatable
);

bool usertypeI_has(morphine_instance_t, const char *name);
struct usertype *usertypeI_get(morphine_instance_t, const char *);
