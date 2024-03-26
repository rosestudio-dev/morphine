//
// Created by whyiskra on 1/21/24.
//

#pragma once

#include <setjmp.h>
#include "morphine/platform.h"
#include "morphine/gc/structure.h"
#include "metatable.h"

struct throw {
    bool inited;
    jmp_buf handler;
    bool is_message;
    union {
        struct value value;
        const char *message;
    } result;
    morphine_state_t cause_state;
};

struct instance {
    struct platform platform;
    struct params params;
    struct require_loader *require_loader_table;
    void *userdata;

    struct garbage_collector G;
    morphine_state_t states;
    morphine_state_t candidates;

    uint16_t interpreter_circle;

    struct table *env;
    struct table *registry;

    struct throw throw;

    struct {
        struct string *names[MFS_COUNT];
        struct table *defaults[VALUE_TYPES_COUNT];
    } metatable;
};

morphine_instance_t instanceI_open(struct platform, struct params, void *userdata);
void instanceI_close(morphine_instance_t);

void instanceI_require_table(morphine_instance_t, struct require_loader *table);
