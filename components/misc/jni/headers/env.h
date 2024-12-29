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

        struct {
            jclass error;
            jclass panic;
        } exception;

        struct {
            jobject object;
            jmethodID call_id;
        } callable;

        struct {
            jclass clazz;

            struct {
                jclass clazz;
                jobject object;
            } nil;

            struct {
                jclass clazz;
                jmethodID constructor_id;
                jfieldID value_id;
            } integer;

            struct {
                jclass clazz;
                jmethodID constructor_id;
                jfieldID value_id;
            } decimal;

            struct {
                jclass clazz;
                jmethodID constructor_id;
                jfieldID value_id;
            } boolean;

            struct {
                jclass clazz;
                jmethodID constructor_id;
                jfieldID value_id;
            } string;

            struct {
                jclass clazz;
                jmethodID constructor_id;
                jfieldID value_id;
            } table;

            struct {
                jclass clazz;
                jmethodID constructor_id;
                jfieldID value_id;
            } vector;
        } value;

        struct {
            jclass clazz;
            jmethodID interrupted_id;
        } thread;

        struct {
            jclass clazz;
            jmethodID constructor_id;
            jmethodID add_id;
            jmethodID get_id;
            jmethodID size_id;
        } array_list;

        struct {
            jclass clazz;
            jmethodID constructor_id;
            jmethodID put_id;
            jmethodID entry_set_id;
        } hash_map;

        struct {
            jmethodID iterator_id;
        } set;

        struct {
            jmethodID has_next_id;
            jmethodID next_id;
        } iterator;

        struct {
            jmethodID get_key_id;
            jmethodID get_value_id;
        } map_entry;
    };

    struct {
        morphine_instance_t I;
        jmp_buf jmp;
    } exit;
};

#define jmp_env(e, success, failure) if (setjmp((e)->exit.jmp) == 1) { if ((e)->exit.I != NULL) { mapi_close((e)->exit.I); { success; } } { failure; } }

struct env env_init(JNIEnv *, jobject);
morphine_platform_t env_platform(void);
morphine_noret void env_exit(morphine_instance_t);
