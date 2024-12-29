//
// Created by why-iskra on 12.11.2024.
//

#pragma once

#include <jni.h>
#include <morphine.h>

#define J(e, n, ...) ((*(e))->n((e), __VA_ARGS__))

size_t jniutils_jint2size(jint);
jsize jniutils_size2jsize(morphine_coroutine_t, size_t, bool *);

jsize jniutils_mlsize2jsize(morphine_coroutine_t, ml_size, bool *);
ml_size jniutils_jsize2mlsize(morphine_coroutine_t, jsize, bool *);

void jniutils_jstring2mlstring(morphine_coroutine_t, JNIEnv *, jstring, bool *);
jstring jniutils_mlstring2jstring(morphine_coroutine_t, JNIEnv *);

bool jniutils_ignore_exception(JNIEnv *);
void jniutils_check_exception(morphine_instance_t, JNIEnv *);

void jniutils_release(JNIEnv *, jobject *, size_t);
