//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"
#include "morphine/misc/packer.h"

struct closure {
    struct object header;

    bool lock;
    struct value callable;
    struct value value;
};

struct closure *closureI_create(morphine_instance_t, struct value callable, struct value value);
void closureI_free(morphine_instance_t, struct closure *);

void closureI_lock(morphine_instance_t, struct closure *);
void closureI_unlock(morphine_instance_t, struct closure *);
struct value closureI_value(morphine_instance_t, struct closure *);

void closureI_packer_vectorize(morphine_instance_t, struct closure *, struct packer_vectorize *);
void closureI_packer_write_info(morphine_instance_t, struct closure *, struct packer_write *);
void closureI_packer_write_data(morphine_instance_t, struct closure *, struct packer_write *);
struct closure *closureI_packer_read_info(morphine_instance_t, struct packer_read *);
void closureI_packer_read_data(morphine_instance_t, struct closure *, struct packer_read *);
