//
// Created by why-iskra on 22.04.2024.
//

#pragma once

#include "morphine/core/value.h"
#include "morphine/misc/packer.h"

struct vector {
    struct object header;

    struct {
        bool fixed;
        bool mutable;
        bool accessible;
        bool locked;
    } mode;

    struct {
        ml_size real;
        ml_size accessible;
    } size;

    struct value *values;
};

struct vector *vectorI_create(morphine_instance_t, ml_size size);
void vectorI_free(morphine_instance_t, struct vector *);

void vectorI_mode_fixed(morphine_instance_t, struct vector *, bool is_fixed);
void vectorI_mode_mutable(morphine_instance_t, struct vector *, bool is_mutable);
void vectorI_mode_accessible(morphine_instance_t, struct vector *, bool is_accessible);
void vectorI_mode_lock(morphine_instance_t, struct vector *);

ml_size vectorI_size(morphine_instance_t, struct vector *);

void vectorI_set(morphine_instance_t, struct vector *, ml_size, struct value);
void vectorI_add(morphine_instance_t, struct vector *, ml_size, struct value);
bool vectorI_has(morphine_instance_t, struct vector *, struct value);
struct value vectorI_get(morphine_instance_t, struct vector *, ml_size);
struct value vectorI_remove(morphine_instance_t, struct vector *, ml_size);

void vectorI_resize(morphine_instance_t, struct vector *, ml_size);
struct vector *vectorI_copy(morphine_instance_t, struct vector *);
struct vector *vectorI_concat(morphine_instance_t, struct vector *, struct vector *);
void vectorI_sort(morphine_instance_t, struct vector *);

struct value vectorI_iterator_first(morphine_instance_t, struct vector *, bool *has);
struct pair vectorI_iterator_next(morphine_instance_t, struct vector *, struct value *key, bool *next);

void vectorI_packer_vectorize(struct vector *, struct packer_vectorize *);
void vectorI_packer_write_info(struct vector *, struct packer_write *);
void vectorI_packer_write_data(struct vector *, struct packer_write *);
struct vector *vectorI_packer_read_info(morphine_instance_t, struct packer_read *);
void vectorI_packer_read_data(morphine_instance_t, struct vector *, struct packer_read *);
