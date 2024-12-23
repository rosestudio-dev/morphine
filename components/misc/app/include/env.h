//
// Created by why-iskra on 24.09.2024.
//

#pragma once

#include <setjmp.h>
#include <morphine.h>

struct env {
    jmp_buf abort_jmp;

    struct {
        int code;
        morphine_instance_t I;
        jmp_buf jmp;
    } exit;
};

morphine_noret void env_signal(morphine_instance_t, void *, bool);
morphine_noret void env_exit(morphine_instance_t, ml_integer code);
