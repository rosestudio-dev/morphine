//
// Created by why-iskra on 24.09.2024.
//

#pragma once

#include <setjmp.h>
#include <morphine.h>
#include <jni.h>

struct env {
    struct {
        JNIEnv *jnienv;
        jobject this;

        struct {
            jobject object;
            jmethodID read_id;
        } input;

        struct {
            jobject object;
            jmethodID write_id;
            jmethodID flush_id;
        } output;

        struct {
            jobject object;
            jmethodID write_id;
            jmethodID flush_id;
        } error;
    };

    struct {
        morphine_instance_t I;
        jmp_buf jmp;
    } exit;
};

#define jmp_env(e, success, failure) if (setjmp((e)->exit.jmp) == 1) { if ((e)->exit.I != NULL) { mapi_close((e)->exit.I); { success; } } { failure; } }

morphine_noret void env_signal(morphine_instance_t);
morphine_noret void env_exit(morphine_instance_t);
