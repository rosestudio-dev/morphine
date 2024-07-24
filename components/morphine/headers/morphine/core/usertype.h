//
// Created by why-iskra on 21.07.2024.
//

#pragma once

#include "morphine/platform.h"

struct usertype;

struct usertype_info {
    const char *name;
    size_t name_len;

    size_t allocate;
    morphine_free_t free;
    bool require_metatable;
};

struct usertypes {
    struct usertype *types;
};

struct usertypes usertypeI_prototype(void);
void usertypeI_free(morphine_instance_t, struct usertypes *);

bool usertypeI_declare(
    morphine_instance_t,
    const char *name,
    size_t allocate,
    morphine_free_t free,
    bool require_metatable
);

bool usertypeI_undeclare(morphine_instance_t, const char *name);

bool usertypeI_is_declared(morphine_instance_t, const char *name);
struct usertype *usertypeI_get(morphine_instance_t, const char *);
struct usertype_info usertypeI_info(morphine_instance_t, struct usertype *);
void usertypeI_ref(morphine_instance_t, struct usertype *);
void usertypeI_unref(morphine_instance_t, struct usertype *);
