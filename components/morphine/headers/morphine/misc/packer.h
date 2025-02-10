//
// Created by why-iskra on 21.08.2024.
//

#pragma once

#include "morphine/core/instruction/type.h"
#include "morphine/object/stream.h"
#include "morphine/platform.h"

#define FORMAT_TAG ("morphine-packed")
#define PACKER_VERSION (1)

#define PROB_INTEGER (-201427)
#define PROB_SIZE    (201427)
#define PROB_DECIMAL (1548.5629)

void packerI_to(morphine_instance_t, struct stream *, struct value);
struct value packerI_from(morphine_instance_t, struct stream *);
