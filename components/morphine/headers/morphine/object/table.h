//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"

struct bucket {
    struct {
        struct bucket *prev;
        struct bucket *next;
    } ll;

    struct {
        struct bucket *prev;
    } tree;

    struct pair pair;
};

struct hashmap {
    struct {
        struct bucket *head;
        struct bucket *tail;
        size_t count;
    } buckets;

    struct {
        struct bucket **trees;
        size_t used;
        size_t size;
    } hashing;
};

struct table {
    struct object header;

    struct table *metatable;

    struct hashmap hashmap;
};

struct table *tableI_create(morphine_instance_t);
void tableI_free(morphine_instance_t, struct table *);

size_t tableI_size(morphine_instance_t, struct table *);
void tableI_set(morphine_instance_t, struct table *, struct value key, struct value value);
struct value tableI_get(morphine_instance_t, struct table *, struct value key, bool *has);
bool tableI_remove(morphine_instance_t, struct table *, struct value key);
void tableI_clear(morphine_instance_t, struct table *);

struct value tableI_first(morphine_instance_t, struct table *, bool *has);
struct pair tableI_next(
    morphine_instance_t,
    struct table *,
    struct value *key,
    bool *next
);
