//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/object.h"

struct userdata {
    struct object header;

    char *type;
    void *userdata;

    morphine_userdata_mark_t mark;
    morphine_userdata_free_t free;

    struct table *metatable;
};

struct userdata *userdataI_create(
    morphine_instance_t,
    const char *type,
    void *p,
    morphine_userdata_mark_t mark,
    morphine_userdata_free_t free
);

void userdataI_free(morphine_instance_t, struct userdata *);

size_t userdataI_allocated_size(struct userdata *);
