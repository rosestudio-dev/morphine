//
// Created by whyiskra on 3/24/24.
//

#pragma once

#include <libcompiler_shared_api.h>
#include "morphine.h"

struct libcompiler {
    libsharedcompiler_ExportedSymbols *symbols;
};

struct compiler_instance;

struct libcompiler libcompiler_open(const char *path);

struct compiler_instance *libcompiler_userdata(morphine_coroutine_t);
bool libcompiler_compile(morphine_coroutine_t, struct compiler_instance *, char *text, bool optimize);
const char *libcompiler_get_error(morphine_coroutine_t, struct compiler_instance *);
int libcompiler_get_bytecode_size(morphine_coroutine_t, struct compiler_instance *);
const uint8_t *libcompiler_get_bytecode(morphine_coroutine_t, struct compiler_instance *);
const char *libcompiler_version(morphine_coroutine_t, struct compiler_instance *);
int libcompiler_version_code(morphine_coroutine_t, struct compiler_instance *);
void libcompiler_release(morphine_coroutine_t, struct compiler_instance *);
