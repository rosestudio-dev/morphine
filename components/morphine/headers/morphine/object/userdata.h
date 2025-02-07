//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/gc/object.h"

struct userdata {
    struct object header;

    bool inited;
    bool is_typed;
    union {
        struct usertype *typed;

        struct {
            size_t size;
            mfunc_destructor_t destructor;
        } untyped;
    };

    void *data;

    struct table *metatable;
};

struct userdata *userdataI_instance(morphine_instance_t, const char *type);
struct userdata *userdataI_create_uni(morphine_instance_t, size_t, mfunc_constructor_t, mfunc_destructor_t);
struct userdata *userdataI_create_vec(morphine_instance_t, size_t, size_t, mfunc_constructor_t, mfunc_destructor_t);
void userdataI_free(morphine_instance_t, struct userdata *);

void *userdataI_pointer(morphine_instance_t, struct userdata *, const char *type);
