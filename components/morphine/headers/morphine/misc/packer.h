//
// Created by why-iskra on 21.08.2024.
//

#pragma once

#include "morphine/misc/instruction/type.h"
#include "morphine/object/stream.h"
#include "morphine/platform.h"

#define FORMAT_TAG ("morphine-packed")
#define PACKER_VERSION (1)

#define PROB_INTEGER (-201427)
#define PROB_SIZE    (201427)
#define PROB_DECIMAL (1548.5629)

struct packer_vectorize;
struct packer_write;
struct packer_read;

void packerI_to(morphine_instance_t, struct stream *, struct value value);
void packerI_vectorize_append(struct packer_vectorize *, struct value);
void packerI_write_ml_size(struct packer_write *, ml_size);
void packerI_write_ml_line(struct packer_write *, ml_line);
void packerI_write_ml_argument(struct packer_write *, ml_argument);
void packerI_write_opcode(struct packer_write *, mtype_opcode_t);
void packerI_write_bool(struct packer_write *, bool);
void packerI_write_object_string(struct packer_write *, struct string *);
void packerI_write_value(struct packer_write *, struct value);

struct value packerI_from(morphine_instance_t, struct stream *);
ml_size packerI_read_ml_size(struct packer_read *);
ml_line packerI_read_ml_line(struct packer_read *);
ml_argument packerI_read_ml_argument(struct packer_read *);
mtype_opcode_t packerI_read_opcode(struct packer_read *);
bool packerI_read_bool(struct packer_read *);
struct string *packerI_read_object_string(struct packer_read *);
struct value packerI_read_value(struct packer_read *);
