//
// Created by whyiskra on 3/24/24.
//

#pragma once

#include "morphine.h"
#include <libcompiler_dynamic.h>

struct compiler_instance;

struct compiler_instance *dlibcompiler_userdata(morphine_state_t, const char *path);

bool dlibcompiler_assemble(morphine_state_t, struct compiler_instance *, char *text, bool optimize);
const char *dlibcompiler_geterror(morphine_state_t, struct compiler_instance *);
int dlibcompiler_getbytecodesize(morphine_state_t, struct compiler_instance *);
const uint8_t *dlibcompiler_getbytecodevector(morphine_state_t, struct compiler_instance *);
const char *dlibcompiler_version(morphine_state_t, struct compiler_instance *);
