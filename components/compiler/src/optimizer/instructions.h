//
// Created by why-iskra on 06.10.2024.
//

#pragma once

#include <morphine.h>
#include "instruction.h"

struct instructions;

struct instructions *instructions_alloc(morphine_coroutine_t);
void instructions_free(morphine_instance_t, struct instructions *);
void instructions_build(morphine_coroutine_t, struct instructions *);
ml_size instructions_size(struct instructions *);
struct instruction instructions_get(morphine_coroutine_t, struct instructions *, ml_size);
