//
// Created by whyiskra on 1/21/24.
//

#pragma once

#include <setjmp.h>
#include "morphine/platform.h"
#include "morphine/gc/structure.h"
#include "morphine/core/interpreter.h"
#include "morphine/core/libraries.h"
#include "morphine/core/usertype.h"
#include "morphine/misc/metatable.h"

struct instance {
    morphine_platform_t platform;
    morphine_settings_t settings;
    void *data;

    struct garbage_collector G;
    struct interpreter E;
    struct libraries libraries;
    struct usertypes usertypes;

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

morphine_instance_t instanceI_open(morphine_platform_t, morphine_settings_t, void *data);
void instanceI_close(morphine_instance_t);
