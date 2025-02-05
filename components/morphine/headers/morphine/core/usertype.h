//
// Created by why-iskra on 21.07.2024.
//

#pragma once

#include "morphine/platform.h"

struct usertype;

struct usertype_info {
    const char *name;

    size_t allocate;
    mfunc_constructor_t constructor;
    mfunc_destructor_t destructor;
    mfunc_compare_t compare;
    mfunc_hash_t hash;
    bool require_metatable;
};

struct usertypes {
    struct usertype *types;
};

struct usertypes usertypeI_prototype(void);
void usertypeI_free(morphine_instance_t, struct usertypes *);

void usertypeI_declare(
    morphine_instance_t,
    const char *name,
    size_t allocate,
    bool require_metatable,
    mfunc_constructor_t,
    mfunc_destructor_t,
    mfunc_compare_t,
    mfunc_hash_t
);

bool usertypeI_is_declared(morphine_instance_t, const char *name);
struct usertype *usertypeI_get(morphine_instance_t, const char *);
struct usertype_info usertypeI_info(morphine_instance_t, struct usertype *);
void usertypeI_ref(morphine_instance_t, struct usertype *);
void usertypeI_unref(morphine_instance_t, struct usertype *);
bool usertypeI_eq(struct usertype *, struct usertype *);
