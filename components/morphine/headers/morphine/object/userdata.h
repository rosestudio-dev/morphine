//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/gc/object.h"

struct userdata {
    struct object header;

    char *name;
    size_t name_len;

    size_t size;
    void *data;

    morphine_mark_t mark;
    morphine_free_t free;

    struct table *metatable;
};

struct userdata *userdataI_create(
    morphine_instance_t, const char *name,
    size_t size,
    morphine_mark_t mark,
    morphine_free_t free
);

struct userdata *userdataI_create_vec(
    morphine_instance_t, const char *name,
    size_t count, size_t size,
    morphine_mark_t mark,
    morphine_free_t free
);

void userdataI_free(morphine_instance_t, struct userdata *);

void userdataI_resize(morphine_instance_t, struct userdata *, size_t);
void userdataI_resize_vec(morphine_instance_t, struct userdata *, size_t count, size_t);
