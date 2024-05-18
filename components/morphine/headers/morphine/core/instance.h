//
// Created by whyiskra on 1/21/24.
//

#pragma once

#include <setjmp.h>
#include "morphine/platform.h"
#include "morphine/gc/structure.h"
#include "morphine/core/interpreter.h"
#include "morphine/misc/metatable.h"

struct instance {
    struct platform platform;
    struct settings settings;
    struct require_loader *require_loader_table;
    void *data;

    struct garbage_collector G;
    struct interpreter E;

    struct table *env;
    struct table *registry;

    struct {
        struct string *names[MFS_COUNT];
        struct table *defaults[VALUE_TYPES_COUNT];
    } metatable;

    struct {
        struct sio *io;
        struct sio *error;
    } sio;
};

morphine_instance_t instanceI_open(struct platform, struct settings, void *data);
void instanceI_close(morphine_instance_t);

void instanceI_require_table(morphine_instance_t, struct require_loader *table);
