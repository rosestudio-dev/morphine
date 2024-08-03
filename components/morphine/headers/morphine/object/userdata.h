//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/gc/object.h"

struct userdata {
    struct object header;

    bool is_typed;
    union {
        struct {
            bool inited;
            struct usertype *usertype;
        } typed;

        struct {
            size_t size;
            morphine_userdata_free_t free;
        } untyped;
    };

    struct {
        bool metatable_locked;
        bool size_locked;
    } mode;

    void *data;

    struct table *metatable;
};

struct userdata *userdataI_instance(morphine_instance_t, const char *type, struct table *metatable);
struct userdata *userdataI_create(morphine_instance_t, size_t size);
struct userdata *userdataI_create_vec(morphine_instance_t, size_t count, size_t size);
void userdataI_free(morphine_instance_t, struct userdata *);

void userdataI_set_free(morphine_instance_t, struct userdata *, morphine_userdata_free_t);

void userdataI_mode_lock_metatable(morphine_instance_t, struct userdata *);
void userdataI_mode_lock_size(morphine_instance_t, struct userdata *);

void userdataI_resize(morphine_instance_t, struct userdata *, size_t);
void userdataI_resize_vec(morphine_instance_t, struct userdata *, size_t count, size_t);
