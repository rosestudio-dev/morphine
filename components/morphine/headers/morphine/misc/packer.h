//
// Created by why-iskra on 21.08.2024.
//

#pragma once

#include "morphine/platform.h"
#include "morphine/object/sio.h"
#include "morphine/misc/instruction/type.h"

#define FORMAT_TAG ("morphine-packed")
#define PACKER_VERSION (1)

#define PROB_INTEGER (-201427)
#define PROB_SIZE    (201427)
#define PROB_DECIMAL (1548.5629)

struct packer_vectorize;
struct packer_write;
struct packer_read;

void packerI_to(morphine_instance_t, struct sio *, struct value value);
void packerI_vectorize_append(struct packer_vectorize *, struct value);
void packerI_write_ml_size(struct packer_write *, ml_size);
void packerI_write_ml_line(struct packer_write *, ml_line);
void packerI_write_ml_argument(struct packer_write *, ml_argument);
void packerI_write_opcode(struct packer_write *, morphine_opcode_t);
void packerI_write_bool(struct packer_write *, bool);
void packerI_write_object_string(struct packer_write *, struct string *);
void packerI_write_value(struct packer_write *, struct value);
morphine_noret void packerI_vectorize_error(struct packer_vectorize *, const char *);
morphine_noret void packerI_write_error(struct packer_write *, const char *);

struct value packerI_from(morphine_instance_t, struct sio *);
ml_size packerI_read_ml_size(struct packer_read *);
ml_line packerI_read_ml_line(struct packer_read *);
ml_argument packerI_read_ml_argument(struct packer_read *);
morphine_opcode_t packerI_read_opcode(struct packer_read *);
bool packerI_read_bool(struct packer_read *);
struct string *packerI_read_object_string(struct packer_read *);
struct value packerI_read_value(struct packer_read *);
