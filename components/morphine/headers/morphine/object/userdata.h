//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/object.h"

struct link {
    struct userdata *userdata;
    bool soft;
    struct link *prev;
};

struct userdata {
    struct object header;

    char *name;
    size_t name_len;

    void *data;

    morphine_userdata_mark_t mark;
    morphine_userdata_free_t free;

    struct table *metatable;
};

struct userdata *userdataI_create(
    morphine_instance_t,
    const char *name,
    void *p,
    morphine_userdata_mark_t mark,
    morphine_userdata_free_t free
);

void userdataI_free(morphine_instance_t, struct userdata *);
