//
// Created by whyiskra on 16.12.23.
//

#pragma once

#include "morphine/core/value.h"
#include "morphine/misc/instruction/type.h"
#include "morphine/misc/packer.h"
#include <stddef.h>

struct function {
    struct object header;

    struct string *name;
    ml_line line;
    ml_size instructions_count;
    ml_size constants_count;
    ml_size slots_count;
    ml_size params_count;

    morphine_instruction_t *instructions;
    struct value *constants;

    ml_size stack_size;
};

struct function *functionI_create(
    morphine_instance_t,
    struct string *name,
    ml_line line,
    ml_size instructions_count,
    ml_size constants_count,
    ml_size slots_count,
    ml_size params_count
);

void functionI_free(morphine_instance_t, struct function *);

morphine_instruction_t functionI_instruction_get(morphine_instance_t, struct function *, ml_size index);
void functionI_instruction_set(morphine_instance_t, struct function *, ml_size index, morphine_instruction_t);

struct value functionI_constant_get(morphine_instance_t, struct function *, ml_size index);
void functionI_constant_set(morphine_instance_t, struct function *, ml_size index, struct value);

struct function *functionI_copy(morphine_instance_t, struct function *);

void functionI_packer_vectorize(morphine_instance_t, struct function *, struct packer_vectorize *);
void functionI_packer_write_info(morphine_instance_t, struct function *, struct packer_write *);
void functionI_packer_write_data(morphine_instance_t, struct function *, struct packer_write *);
struct function *functionI_packer_read_info(morphine_instance_t, struct packer_read *);
void functionI_packer_read_data(morphine_instance_t, struct function *, struct packer_read *);
