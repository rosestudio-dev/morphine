//
// Created by why-iskra on 21.07.2024.
//

#pragma once

#include "morphine/platform.h"

struct usertype;

struct usertype_info {
    const char *name;

    size_t allocate;
    morphine_userdata_constructor_t constructor;
    morphine_userdata_destructor_t destructor;
    morphine_userdata_compare_t compare;
    morphine_userdata_hash_t hash;
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
    morphine_userdata_constructor_t,
    morphine_userdata_destructor_t,
    morphine_userdata_compare_t,
    morphine_userdata_hash_t
);

bool usertypeI_is_declared(morphine_instance_t, const char *name);
struct usertype *usertypeI_get(morphine_instance_t, const char *);
struct usertype_info usertypeI_info(morphine_instance_t, struct usertype *);
void usertypeI_ref(morphine_instance_t, struct usertype *);
void usertypeI_unref(morphine_instance_t, struct usertype *);
bool usertypeI_eq(struct usertype *, struct usertype *);
