//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"
#include "morphine/misc/packer.h"

struct closure {
    struct object header;

    ml_size size;
    struct value callable;
    struct value *values;
};

struct closure *closureI_create(morphine_instance_t, struct value callable, ml_size);
void closureI_free(morphine_instance_t, struct closure *);

struct value closureI_get(morphine_instance_t, struct closure *, ml_size);
void closureI_set(morphine_instance_t, struct closure *, ml_size, struct value);

void closureI_packer_vectorize(morphine_instance_t, struct closure *, struct packer_vectorize *);
void closureI_packer_write_info(morphine_instance_t, struct closure *, struct packer_write *);
void closureI_packer_write_data(morphine_instance_t, struct closure *, struct packer_write *);
struct closure *closureI_packer_read_info(morphine_instance_t, struct packer_read *);
void closureI_packer_read_data(morphine_instance_t, struct closure *, struct packer_read *);
