//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"
#include "morphine/misc/packer.h"

struct pair {
    struct value key;
    struct value value;
};

enum bucket_color {
    BUCKET_COLOR_BLACK,
    BUCKET_COLOR_RED,
};

struct bucket {
    struct {
        ml_size index;
        struct bucket *prev;
        struct bucket *next;
    } ll;

    struct bucket *left;
    struct bucket *right;
    struct bucket *parent;
    enum bucket_color color;
    struct pair pair;
};

struct tree {
    struct bucket nil_leaf;
    struct bucket root;
};

struct hashmap {
    struct {
        struct bucket *access;
        struct bucket *head;
        struct bucket *tail;
        ml_size count;
    } buckets;

    struct {
        struct tree *trees;
        size_t used;
        size_t size;
    } hashing;
};

struct table {
    struct object header;

    struct table *metatable;

    struct {
        bool fixed;
        bool mutable;
        bool accessible;
    } mode;

    struct {
        bool mode;
    } lock;

    struct hashmap hashmap;
};

struct table *tableI_create(morphine_instance_t);
void tableI_free(morphine_instance_t, struct table *);

void tableI_mode_fixed(morphine_instance_t, struct table *, bool is_fixed);
void tableI_mode_mutable(morphine_instance_t, struct table *, bool is_mutable);
void tableI_mode_accessible(morphine_instance_t, struct table *, bool is_accessible);
void tableI_lock_mode(morphine_instance_t, struct table *);

ml_size tableI_size(morphine_instance_t, struct table *);
void tableI_set(morphine_instance_t, struct table *, struct value key, struct value value);
bool tableI_has(morphine_instance_t, struct table *, struct value key);
struct value tableI_get(morphine_instance_t, struct table *, struct value key, bool *has);
struct value tableI_remove(morphine_instance_t, struct table *, struct value key, bool *has);

void tableI_idx_set(morphine_instance_t, struct table *, ml_size, struct value);
struct pair tableI_idx_get(morphine_instance_t, struct table *, ml_size, bool *has);

struct pair tableI_first(morphine_instance_t, struct table *, bool *has);
struct pair tableI_next(morphine_instance_t, struct table *, struct value, bool *has);

void tableI_clear(morphine_instance_t, struct table *);
struct table *tableI_concat(morphine_instance_t, struct table *, struct table *);
struct table *tableI_copy(morphine_instance_t, struct table *);

void tableI_packer_vectorize(morphine_instance_t, struct table *, struct packer_vectorize *);
void tableI_packer_write_info(morphine_instance_t, struct table *, struct packer_write *);
void tableI_packer_write_data(morphine_instance_t, struct table *, struct packer_write *);
struct table *tableI_packer_read_info(morphine_instance_t, struct packer_read *);
void tableI_packer_read_data(morphine_instance_t, struct table *, struct packer_read *);
