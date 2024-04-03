//
// Created by whyiskra on 1/21/24.
//

#pragma once

#include <setjmp.h>
#include "morphine/platform.h"
#include "morphine/gc/structure.h"
#include "morphine/core/throw.h"
#include "metatable.h"

struct instance {
    struct platform platform;
    struct settings settings;
    struct require_loader *require_loader_table;
    void *userdata;

    struct garbage_collector G;
    struct throw throw;

    morphine_state_t states;
    morphine_state_t candidates;

    uint16_t interpreter_circle;

    struct table *env;
    struct table *registry;

    struct {
        struct string *names[MFS_COUNT];
        struct table *defaults[VALUE_TYPES_COUNT];
    } metatable;
};

morphine_instance_t instanceI_open(struct platform, struct settings, void *userdata);
void instanceI_close(morphine_instance_t);

void instanceI_require_table(morphine_instance_t, struct require_loader *table);
